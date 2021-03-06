#include "Service.h"
#include "NetworkErrorCode.h"
#include <openssl/err.h>

namespace Netcode::Network {

	static uint32_t NcGetSerializedSize(const Protocol::Control * control, uint32_t payloadSize) {
		if(control->type() == Protocol::MessageType::PMTU_DISCOVERY) {
			return payloadSize;
		}
		return static_cast<uint32_t>(control->ByteSizeLong()) + 1;
	}

	static ArrayView<uint8_t> NcSerialize(NetAllocator * allocator, const Protocol::Control * control, uint32_t payloadSize) {
		const uint32_t serializedSize = NcGetSerializedSize(control, payloadSize);
		const uint32_t alignedSize = Utility::Align<uint32_t, 16>(serializedSize);
		uint8_t * data = allocator->MakeArray<uint8_t>(alignedSize);

		google::protobuf::io::ArrayOutputStream aos{ data, static_cast<int32_t>(serializedSize) };
		google::protobuf::io::CodedOutputStream codedStream{ &aos };

		data[0] = 64;
		codedStream.Skip(1);

		if(!control->SerializeToCodedStream(&codedStream)) {
			return ArrayView<uint8_t>{};
		}

		int byteCount = codedStream.ByteCount();

		if(control->type() == Protocol::PMTU_DISCOVERY) {
			memset(data + byteCount, '<', payloadSize - byteCount);
		}

		return ArrayView<uint8_t>{ data, serializedSize };
	}

	static bool NeedToAck(const Protocol::Control * control) {
		return (control->type() & 0x1) == 0x1;
	}
	
	NetcodeService::ParseResult NetcodeService::HandleAuthenticatedMessage(NetAllocator * alloc, Ref<ConnectionBase> conn, UdpPacket * pkt) {
		post(conn->strand, [this, c = conn, al = alloc->shared_from_this(), pkt]() mutable -> void {
			MutableArrayView<uint8_t> destView{ pkt->GetData() + MtuValue::DTLS_RL_HEADER_SIZE, pkt->GetCapacity() - MtuValue::DTLS_RL_HEADER_SIZE };
			const std::error_code ec = SslReceive(c->dtlsRoute->ssl.get(),
				destView,
				ArrayView<uint8_t>{ pkt->GetData(), pkt->GetSize() });

			if(!ec) {
				if(destView.Size() == 0) {
					return;
				}
				
				if(destView.Data()[0] >= 128) {
					GameFragment * frag = al->Make<GameFragment>();
					frag->packet = pkt;
					frag->content = destView.Data();
					frag->contentSize = static_cast<uint32_t>(destView.Size());
					frag->record = DtlsRecordLayer::Load(reinterpret_cast<const DtlsRecordLayerWire *>(pkt->GetData()));
					frag->header = NcGameHeader::Load(reinterpret_cast<const NcCommonHeaderWire *>(pkt->GetData() + MtuValue::DTLS_RL_HEADER_SIZE));
					GameMessage gMsg = c->fragmentStorage.AddFragment(std::move(al), frag);

					if(gMsg.allocator != nullptr) {
						Node<GameMessage> * node = gMsg.allocator->Make<Node<GameMessage>>();
						node->sequence = gMsg.sequence;
						node->content = gMsg.content;
						node->allocator = std::move(gMsg.allocator);
						c->sharedQueue.Produce(node);
					}
					
					return;
				}

				Protocol::Control * control = ReceiveControl(al.get(), c->dtlsRoute, pkt, destView);

				if(control != nullptr) {
					Node<ControlMessage> * node = al->Make<Node<ControlMessage>>();
					node->allocator = std::move(al);
					node->control = control;
					node->packet = pkt;
					c->sharedControlQueue.Produce(node);
				}
			}
		});
		return ParseResult::TOOK_OWNERSHIP;
	}
	
	Protocol::Control * NetcodeService::ReceiveControl(NetAllocator * alloc, DtlsRoute * route, UdpPacket* pkt, ArrayView<uint8_t> source)
	{
		Protocol::Control * control = alloc->MakeProto<Protocol::Control>();

		if(source.Size() == 0)
			return nullptr;

		const uint8_t * pData = source.Data();
		int32_t dataSize = static_cast<int32_t>(source.Size()) - 1;
		
		if(pData[0] != 64)
			return nullptr;

		std::string_view view{ reinterpret_cast<const char *>(pData + 1), static_cast<size_t>(dataSize) };
		size_t indexOf = view.find_first_of('<');
		if(indexOf != std::string::npos)
			dataSize = static_cast<int32_t>(indexOf);
		
		if(!control->ParseFromArray(pData + 1, dataSize))
			return nullptr;

		const AckClassification ackClass = (route != nullptr) ?
			AckClassification::EXTERNAL_SECURE :
			AckClassification::EXTERNAL_INSECURE;
		
		if(control->type() == Protocol::MessageType::ACKNOWLEDGE) {
			pendingTokenStorage.Ack(control->sequence(), pkt->GetEndpoint(), ackClass);
			return nullptr;
		}

		if(route == nullptr) {
			const auto type = control->type();
			if(type != Protocol::MessageType::PMTU_DISCOVERY && type != Protocol::MessageType::CONNECT_PUNCHTHROUGH) {
				return nullptr;
			}
		}
		
		if(NeedToAck(control))
			SendAck(alloc, route, pkt, control->sequence());

		pkt->SetSequence(control->sequence());
		
		return control;
	}

	void NetcodeService::SendAck(NetAllocator* alloc, DtlsRoute * route, UdpPacket * pkt, uint32_t seq)
	{
		Protocol::Control * control = alloc->MakeProto<Protocol::Control>();
		control->set_sequence(seq);
		control->set_type(Protocol::MessageType::ACKNOWLEDGE);

		ArrayView<uint8_t> serialized = NcSerialize(alloc, control, 256);

		if(route != nullptr) {
			MutableArrayView<uint8_t> dst{
				alloc->MakeArray<uint8_t>(256),
				256
			};
			
			if(!SslSend(route->ssl.get(), dst, serialized)) {
				boost::asio::const_buffer cb{ dst.Data(), dst.Size() };
				socket.Send(cb, route->endpoint, [lt = alloc->shared_from_this()](ErrorCode, size_t) -> void { });
			}
		} else {
			boost::asio::const_buffer cb{ serialized.Data(), serialized.Size() };
			socket.Send(cb, pkt->GetEndpoint(), [lt = alloc->shared_from_this()](ErrorCode, size_t) -> void { });
		}
	}

	void NetcodeService::ApplyFilters(const std::vector<std::unique_ptr<FilterBase>> & filters, DtlsRoute* route, ControlMessage & cm) {
		for(const std::unique_ptr<FilterBase>& f: filters) {
			if(!f->IsCompleted()) {
				const FilterResult r = f->Run(this, route, cm);

				if(r == FilterResult::CONSUMED) {
					break;
				}
			}
		}
	}

	void NetcodeService::CheckFilterCompletion(std::vector<std::unique_ptr<FilterBase>> & fltrs) {
		Timestamp ts = SystemClock::LocalNow();
		
		auto it = std::remove_if(std::begin(fltrs), std::end(fltrs), [ts](const std::unique_ptr<FilterBase> & f) -> bool {
			return f->IsCompleted() || f->CheckTimeout(ts);
		});

		fltrs.erase(it, std::end(fltrs));
	}

	void NetcodeService::RunFilters() {
		dtls.AsyncCheckTimeouts();

		Node<NoAuthControlMessage> * head = controlQueue.ConsumeAll();

		for(Node<NoAuthControlMessage>* it = head; it != nullptr;) {
			ApplyFilters(filters, it->route, *it);

			Node<NoAuthControlMessage> * tmp = it->next;
			it->allocator.reset();
			it = tmp;
		}

		CheckFilterCompletion(filters);
	}

	NetcodeService::ParseResult NetcodeService::HandleRoutedMessage(NetAllocator * alloc, DtlsRoute* route, UdpPacket* pkt) {
		if(!SslReceive(route->ssl.get(), pkt)) {
			
			Protocol::Control * control = ReceiveControl(alloc, route, pkt);

			if(control != nullptr) {
				Node<NoAuthControlMessage> * nocm = alloc->Make<Node<NoAuthControlMessage>>();
				nocm->route = route;
				nocm->control = control;
				nocm->allocator = alloc->shared_from_this();
				nocm->packet = pkt;
				controlQueue.Produce(nocm);
				return ParseResult::TOOK_OWNERSHIP;
			}
		}
		return ParseResult::FAILED;
	}

	NetcodeService::ParseResult NetcodeService::TryParseMessage(NetAllocator * alloc, UdpPacket * pkt) {
		if(pkt->GetSize() < 1) {
			return ParseResult::FAILED;
		}

		uint8_t * data = pkt->GetData();

		Protocol::Control * control = ReceiveControl(alloc, nullptr, pkt);

		if(control != nullptr) {
			if(NeedToAck(control)) {
				return ParseResult::TOOK_OWNERSHIP;
			}
			return ParseResult::COMPLETED;
		}

		if(pkt->GetSize() < MtuValue::DTLS_RL_HEADER_SIZE) {
			// not a DTLS header for sure
			return ParseResult::FAILED;
		}
		
		Ref<ConnectionBase> conn = connectionStorage.GetConnectionByEndpoint(pkt->GetEndpoint());

		if(conn != nullptr) {
			return HandleAuthenticatedMessage(alloc, std::move(conn), pkt);
		}

		dtls.AsyncHandlePacket(this, alloc, pkt);

		return ParseResult::TOOK_OWNERSHIP;
	}

	class DtlsFragmentationContext : public std::enable_shared_from_this<DtlsFragmentationContext> {
		uint64_t mtu;
		uint64_t dataOffset;
		ArrayView<uint8_t> view;
		ssl_ptr<BIO> bioLifetime;
		NetcodeService::NetcodeSocketType * socket;
		UdpEndpoint endpoint;
		CompletionToken<TrResult> token;
	public:
		DtlsFragmentationContext(NetcodeService::NetcodeSocketType * s, CompletionToken<TrResult> t, const UdpEndpoint & ep, ArrayView<uint8_t> dataView, ssl_ptr<BIO> wbioLifetime, uint32_t effectiveMtu) {
			mtu = effectiveMtu;
			dataOffset = 0;
			view = dataView;
			bioLifetime = std::move(wbioLifetime);
			socket = s;
			endpoint = ep;
			token = std::move(t);
		}

		void SendFragment() {
			uint64_t sendSizeCandidate = 0;
			do {
				const uint64_t offset = dataOffset + sendSizeCandidate;
				const DtlsRecordLayer recordLayer = DtlsRecordLayer::Load(reinterpret_cast<const DtlsRecordLayerWire *>(view.Data() + offset));

				const uint64_t sendSize = recordLayer.length + DTLS1_RT_HEADER_LENGTH;

				// can we afford to include this?
				if((sendSizeCandidate + sendSize) > mtu) {
					if(sendSizeCandidate == 0) {
						// we are empty handed here, could raise an exception too
						return;
					}
					break; // no
				}

				sendSizeCandidate += sendSize;

				const uint64_t wouldBeDataOffset = dataOffset + sendSizeCandidate;

				// programmer or OpenSSL error
				if(view.Size() < wouldBeDataOffset) {
					Log::Debug("Invalid Record Layer?");
					if(token != nullptr) {
						token->Set(TrResult{ make_error_code(NetworkErrc::BAD_MESSAGE) });
					}
					return;
				}

				if(view.Size() == wouldBeDataOffset) {
					break; // out of stuff to send
				}
			} while(true);

			boost::asio::const_buffer sendRange{
				view.Data() + dataOffset,
				sendSizeCandidate
			};

			dataOffset += sendSizeCandidate;

			socket->Send(sendRange, endpoint, [this, lt = shared_from_this()](const ErrorCode & ec, size_t n) -> void {
				if(ec) {
					if(token != nullptr) {
						token->Set(TrResult{ ec });
					}
				} else {
					if(dataOffset < view.Size()) {
						SendFragment();
					} else {
						if(token != nullptr) {
							token->Set(TrResult{ ec, view.Size() });
						}
					}
				}
			});
		}
	};

	CompletionToken<TrResult> NetcodeService::Send(Ref<NetAllocator> allocator, CompletionToken<TrResult> ct, const UdpEndpoint & endpoint, ssl_ptr<BIO> wbio, uint16_t mtu) {
		BUF_MEM * bm;
		BIO_get_mem_ptr(wbio.get(), &bm);

		const DtlsRecordLayer recordLayer = DtlsRecordLayer::Load(reinterpret_cast<const DtlsRecordLayerWire *>(bm->data));

		if(recordLayer.contentType == DtlsContentType::HANDSHAKE) {
			uint32_t udpMtu = MtuValue{ mtu }.GetUdpPayloadSize(endpoint.address());
			if(bm->length > udpMtu) {
				Ref<DtlsFragmentationContext> fragCtx;
				ArrayView<uint8_t> arrayView{ reinterpret_cast<const uint8_t *>(bm->data), bm->length };
				if(allocator != nullptr) {
					fragCtx = allocator->MakeShared<DtlsFragmentationContext>(&socket, ct, endpoint, arrayView, std::move(wbio), udpMtu);
				} else {
					fragCtx = std::make_shared<DtlsFragmentationContext>(&socket, ct, endpoint, arrayView, std::move(wbio), udpMtu);
				}
				fragCtx->SendFragment();
				return ct;
			}
		}

		boost::asio::const_buffer constBuffer{ bm->data , bm->length };
		socket.Send(constBuffer, endpoint, [ct, w = wbio.release()](const ErrorCode & ec, size_t s) -> void {
			ssl_ptr<BIO> wb{ w };
			if(ct != nullptr) {
				ct->Set(TrResult{ ec, s });
			}
		});

		return ct;
	}

	CompletionToken<TrResult> NetcodeService::Send(Ref<NetAllocator> allocator,
		CompletionToken<TrResult> ct,
		const DtlsRoute * route,
		const ControlMessage & controlMessage,
		const UdpEndpoint & endpoint,
		MtuValue pmtu,
		ResendArgs args) {
		
		const Protocol::MessageType type = controlMessage.control->type();

		if(route == nullptr) {
			// can only send ACK, PUNCHTHROUGH or PMTU_DISCOVERY without encryption
			if(type != Protocol::MessageType::ACKNOWLEDGE &&
				type != Protocol::MessageType::CONNECT_PUNCHTHROUGH &&
				type != Protocol::MessageType::PMTU_DISCOVERY) {
				// cant send the message unencrypted
				if(ct != nullptr) {
					ct->Set(TrResult{ make_error_code(NetworkErrc::UNAUTHORIZED) });
				}
				return ct;
			}
		}
		
		uint32_t payloadSize = pmtu.GetUdpPayloadSize(endpoint.address());
		
		if(route != nullptr) {
			payloadSize -= MtuValue::DTLS_RL_HEADER_SIZE;
			payloadSize = payloadSize - payloadSize % 16;
		}
		
		ArrayView<uint8_t> serializedMessage = NcSerialize(allocator.get(), controlMessage.control, payloadSize);

		UdpPacket * packet = allocator->MakeUdpPacket(serializedMessage.Size() + 128);
		packet->SetEndpoint(endpoint);
		packet->SetSequence(controlMessage.control->sequence());
		
		if((type & 0x1) == 0x1) {
			AckClassification ackClass = (route == nullptr) ? AckClassification::EXTERNAL_INSECURE : AckClassification::EXTERNAL_SECURE;
			WaitableTimer * timer = allocator->Make<WaitableTimer>(ioContext);
			PendingTokenNode * node = allocator->Make<PendingTokenNode>(ct, timer, packet, ackClass);

			Ref<ResendContext<NetcodeSocketType>> rc = allocator->MakeShared<ResendContext<NetcodeSocketType>>(
				&pendingTokenStorage, &socket, ct, serializedMessage, packet, route, timer, args.resendInterval, args.maxAttempts);

			pendingTokenStorage.AddNode(node);

			rc->Attempt();
		} else {

			if(route != nullptr) {
				const ErrorCode ec = SslSend(route->ssl.get(), packet, serializedMessage);

				if(ec) {
					if(ct != nullptr) {
						ct->Set(TrResult{ ec });
					}
					return ct;
				}
			}
			
			socket.Send(packet->GetConstBuffer(), packet->GetEndpoint(), [ct](const ErrorCode & ec, size_t s) -> void {
				if(ct != nullptr) {
					ct->Set(TrResult{ ec, s });
				}
			});
		}

		return ct;
	}

	CompletionToken<TrResult> NetcodeService::Send(const GameMessage & gMsg, ConnectionBase * connection)
	{
		Ref<NetAllocator> allocator = gMsg.allocator;
		const ArrayView<uint8_t> update = gMsg.content;
		const uint32_t sequence = gMsg.sequence;
		const UdpEndpoint endpoint = connection->endpoint;
		const IpAddress address = endpoint.address();
		const MtuValue pmtu = connection->pmtu;
		
		const uint32_t dataSize = static_cast<uint32_t>(update.Size());
		const uint32_t dtlsPayloadSize = pmtu.GetDtlsPayloadSize(address);
		const uint32_t encryptedPayloadSize = GetEncryptedPayloadSize(connection->dtlsRoute->ssl.get(), dtlsPayloadSize) - NC_HEADER_SIZE;
		const uint32_t numFragments = (dataSize + encryptedPayloadSize - 1) / encryptedPayloadSize;
		const uint32_t wireSize = pmtu.GetUdpPayloadSize(address) * numFragments;

		CompletionToken<TrResult> ct = allocator->MakeCompletionToken<TrResult>();

		if(numFragments == 0) {
			ct->Set(TrResult{ make_error_code(NetworkErrc::BAD_MESSAGE) });
			return ct;
		}

		if(numFragments > 256) {
			ct->Set(TrResult{ make_error_code(NetworkErrc::MESSAGE_TOO_BIG) });
			return ct;
		}

		const uint32_t baseOffset = 128 * numFragments;

		UdpPacket * packet = allocator->MakeUdpPacket(Utility::Align<uint32_t, 16>(wireSize + baseOffset));
		uint8_t * pData = packet->GetData();

		memcpy(pData + baseOffset, update.Data(), dataSize);

		uint32_t sourceOffset = baseOffset - NC_HEADER_SIZE;
		uint32_t destOffset = 0;
		uint32_t handledDataSize = 0;

		for(uint32_t i = 0; i < numFragments; i++) {
			NcGameHeader gameHeader;
			gameHeader.sequence = sequence;
			gameHeader.fragmentCount = static_cast<uint16_t>(numFragments);
			gameHeader.fragmentIdx = static_cast<uint16_t>(i);
			gameHeader.Store(reinterpret_cast<NcCommonHeaderWire *>(pData + sourceOffset));

			uint32_t fragmentedDataSize = std::min(dataSize - handledDataSize, encryptedPayloadSize);

			MutableArrayView<uint8_t> dataDestView{
				pData + destOffset,
				packet->GetCapacity() - destOffset
			};

			ArrayView<uint8_t> sourceView{
				pData + sourceOffset,
				fragmentedDataSize + NC_HEADER_SIZE
			};

			ErrorCode ec = SslSend(connection->dtlsRoute->ssl.get(), dataDestView, sourceView);

			if(ec) {
				ct->Set(TrResult{ make_error_code(NetworkErrc::BAD_MESSAGE) });
				return ct;
			}

			destOffset += dataDestView.Size();
			sourceOffset += fragmentedDataSize;
			handledDataSize += fragmentedDataSize;
		}

		Ref<DtlsFragmentationContext> ctx = allocator->MakeShared<DtlsFragmentationContext>(
			&socket, ct, endpoint, ArrayView<uint8_t>{pData, destOffset}, nullptr, mtu.GetDtlsPayloadSize(address));

		ctx->SendFragment();

		return ct;
	}
}
