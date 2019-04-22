create database if not exists d_im;

use d_im;

CREATE TABLE IF NOT EXISTS `t_user` (
    `userId` BIGINT UNSIGNED PRIMARY KEY
);

CREATE TABLE IF NOT EXISTS `t_message` (
    `destId` BIGINT UNSIGNED NOT NULL,
    `srcId` BIGINT UNSIGNED NOT NULL,
    `content` varchar(1024),
    FOREIGN KEY (destId) 
        REFERENCES t_user(userId)
        ON DELETE CASCADE
);
