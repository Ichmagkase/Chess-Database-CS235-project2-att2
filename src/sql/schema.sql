USE chess;
CREATE TABLE Games(
  game_number INT NOT NULL,
  game_result VARCHAR(16),
  ply_count INT,
  PRIMARY KEY(game_number)
);
CREATE TABLE Moves(
  game_number INT NOT NULL,
  ply_number INT NOT NULL,
  piece CHAR(1),
  origin_square CHAR(2),
  dest_square CHAR(2),
  captured_piece CHAR(1),
  promotion CHAR(1),
  is_castle TINYINT(1) DEFAULT 0,
  is_check TINYINT(1) DEFAULT 0,
  is_checkmate TINYINT(1) DEFAULT 0,
  fen VARCHAR(255),
  PRIMARY KEY(
    game_number,
    ply_number
  ),
  FOREIGN KEY(game_number) REFERENCES Games(game_number)
);
CREATE TABLE MetadataTags(
  game_number INT NOT NULL,
  tag_name VARCHAR(255) NOT NULL,
  tag_value VARCHAR(255),
  PRIMARY KEY(
    game_number,
    tag_name
  ),
  FOREIGN KEY(game_number) REFERENCES Games(game_number)
);
