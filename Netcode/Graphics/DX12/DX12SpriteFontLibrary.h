#pragma once

#include "../../HandleTypes.h"
#include "../../Path.h"
#include "DX12Common.h"
#include "DX12SpriteFont.h"
#include <algorithm>

namespace Netcode::Graphics::DX12 {

	class SpriteFontLibrary {

		struct Item {
			DX12SpriteFontRef font;
			std::wstring path;

			Item(DX12SpriteFontRef font, const std::wstring & p) : font{ std::move(font) }, path{ p } { }
		};

		std::vector<Item> items;

	public:
		IResourceContext * resourceCtx;
		IFrameContext * frameCtx;


		DX12SpriteFontRef Get(const std::wstring & mediaPath) {
			Netcode::MediaPath fontPath{ mediaPath };

			auto it = std::find_if(std::begin(items), std::end(items), [&mediaPath](const Item & item) -> bool {
				return item.path == mediaPath;
			});

			if(it == std::end(items)) {
				auto spriteFont = std::make_shared<DX12SpriteFont>(resourceCtx, frameCtx, fontPath.GetAbsolutePath().c_str());

				items.emplace_back(Item{ spriteFont, fontPath.GetAbsolutePath() });

				return spriteFont;
			}

			return it->font;
		}

	};

	using DX12SpriteFontLibrary = Netcode::Graphics::DX12::SpriteFontLibrary;
	using DX12SpriteFontLibraryRef = std::shared_ptr<DX12SpriteFontLibrary>;

}