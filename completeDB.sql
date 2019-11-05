CREATE DATABASE Casado_Zuniga_Project2;

USE Casado_Zuniga_Project2;

CREATE TABLE web_auth (
  `id` BIGINT(10) NOT NULL AUTO_INCREMENT,
  `email` VARCHAR(255) NOT NULL UNIQUE,
  `salt` CHAR(64) NOT NULL,
  `hashed_password` CHAR(64) NOT NULL,
  `userId` BIGINT(10) NOT NULL UNIQUE,
  PRIMARY KEY (`id`))
COMMENT = 'Web Authentication Table';

CREATE TABLE user (
  `id` BIGINT(10) NOT NULL AUTO_INCREMENT,
  `last_login` TIMESTAMP(5) NOT NULL,
  `creation_date` DATETIME(3) NOT NULL,
  PRIMARY KEY (`id`));
