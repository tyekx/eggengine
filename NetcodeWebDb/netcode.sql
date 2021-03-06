-- MySQL dump 10.13  Distrib 8.0.19, for Win64 (x86_64)
--
-- Host: localhost    Database: netcode
-- ------------------------------------------------------
-- Server version	8.0.19

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `game_servers`
--

DROP TABLE IF EXISTS `game_servers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `game_servers` (
  `id` int NOT NULL AUTO_INCREMENT,
  `owner_id` int NOT NULL,
  `max_players` int NOT NULL,
  `interval` int NOT NULL,
  `status` int NOT NULL,
  `hostname` varchar(64) DEFAULT NULL,
  `server_ip` varchar(64) NOT NULL,
  `control_port` int NOT NULL,
  `game_port` int NOT NULL,
  `created_at` timestamp(6) NOT NULL,
  `closed_at` timestamp(6) NULL DEFAULT NULL,
  `version_major` int NOT NULL,
  `version_minor` int NOT NULL,
  `version_build` int NOT NULL,
  PRIMARY KEY (`id`),
  KEY `owner_id` (`owner_id`),
  CONSTRAINT `game_servers_ibfk_1` FOREIGN KEY (`owner_id`) REFERENCES `users` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=163 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `game_servers`
--

LOCK TABLES `game_servers` WRITE;
/*!40000 ALTER TABLE `game_servers` DISABLE KEYS */;
INSERT INTO `game_servers` VALUES (162,1,4,500,2,NULL,'::1',8889,8888,'2020-09-14 09:40:42.247746',NULL,0,0,0);
/*!40000 ALTER TABLE `game_servers` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `game_sessions`
--

DROP TABLE IF EXISTS `game_sessions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `game_sessions` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user_id` int NOT NULL,
  `game_server_id` int NOT NULL,
  `joined_at` timestamp(6) NOT NULL,
  `left_at` timestamp(6) NOT NULL DEFAULT '0000-00-00 00:00:00.000000',
  PRIMARY KEY (`id`),
  UNIQUE KEY `game_sessions_chk_activity` (`user_id`,`left_at`),
  KEY `game_sessions_ibfk_2` (`game_server_id`),
  CONSTRAINT `game_sessions_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`),
  CONSTRAINT `game_sessions_ibfk_2` FOREIGN KEY (`game_server_id`) REFERENCES `game_servers` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=288 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `game_sessions`
--

LOCK TABLES `game_sessions` WRITE;
/*!40000 ALTER TABLE `game_sessions` DISABLE KEYS */;
INSERT INTO `game_sessions` VALUES (286,1,162,'2020-09-14 09:41:40.745145','0000-00-00 00:00:00.000000');
/*!40000 ALTER TABLE `game_sessions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `sessions`
--

DROP TABLE IF EXISTS `sessions`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `sessions` (
  `id` int NOT NULL AUTO_INCREMENT,
  `user_id` int NOT NULL,
  `hash` varchar(64) NOT NULL,
  `expires_at` timestamp(6) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`),
  CONSTRAINT `sessions_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `sessions`
--

LOCK TABLES `sessions` WRITE;
/*!40000 ALTER TABLE `sessions` DISABLE KEYS */;
INSERT INTO `sessions` VALUES (4,1,'43a64a305b3d41b75eabcae27e8dac8e2c8d9a5f0c356fcf8d3c051fbfdd903c','2020-09-14 09:53:17.648049'),(5,1,'de14a05b71fc3910cc34998f358d9ce1d7012e8919093f8817f7cf63507748b8','2020-09-28 09:53:31.467247');
/*!40000 ALTER TABLE `sessions` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `users` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(16) NOT NULL,
  `password` varchar(64) NOT NULL,
  `salt` varchar(64) NOT NULL,
  `is_banned` tinyint NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=14 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES (1,'Tyekx','0823c84ed61b458956dd32ead641d7c9728f8e3df32b39fb4c7a226e2932cee1','9bc537bca29ff4eeadc31a3310a05b543af72c2c5a44e0356e1dcfd299b993dc',0),(11,'Test1','4e396ef180333d4cb3fc5d5a994ffc50c3cae85245e312ab36b19a787b967c91','0758a20c832b9a4e60295879a3c55abad16ae3fee71e942d29bc3dca941d9837',0),(12,'Test2','fbce83c96efc5536c4f7cefcd14b2579c22606e6e2fbcb9b337f4fb5316f8314','d8e659d596c275185fb8bf15012c570a31585d715b608dfccd17a6d90fa406f6',0),(13,'Test3','fa12e54b2605d9c4f07dc4959dad0fc7e9d7d82c33425e1a9ce09cdea4f5ca7f','ec899530516bb11a169749d5a16024a881967aafc27c3425bee5f9b1a4a78812',0);
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2020-09-14 11:58:18
