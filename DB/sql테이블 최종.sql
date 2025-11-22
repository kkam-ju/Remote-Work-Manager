-- 1. 데이터베이스 생성 (이미 있다면 생략)
CREATE schema IF NOT EXISTS yolo_webapp_db CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;

-- 2. 생성된 데이터베이스 사용
USE yolo_webapp_db;

-- 3. employees 테이블 생성
CREATE TABLE employees (
    employee_id      INT              PRIMARY KEY AUTO_INCREMENT,
    employee_number VARCHAR(50)      NOT NULL UNIQUE,
    full_name        VARCHAR(50)      NOT NULL,
    department       VARCHAR(100)     NOT NULL,
    is_admin         TINYINT(1)       NOT NULL DEFAULT 0, -- 0은 FALSE, 1은 TRUE
    is_registered    TINYINT(1)       NOT NULL DEFAULT 0  -- 0은 FALSE, 1은 TRUE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 4. users 테이블 생성 (테이블 이름: users)
CREATE TABLE users (
    user_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '사용자 계정 고유 ID',
    employee_id INT UNIQUE NOT NULL COMMENT 'employees 테이블을 참조하는 Foreign Key',
    username VARCHAR(255) UNIQUE NOT NULL COMMENT '로그인 시 사용할 아이디',
    password VARCHAR(255) NOT NULL COMMENT '해시 처리된 비밀번호',
    email VARCHAR(255) UNIQUE COMMENT '이메일 주소',
    phone_number VARCHAR(255) COMMENT '전화번호',
    ip_address VARCHAR(255) COMMENT '카메라 스트리밍 장치의 IP 주소',
    status VARCHAR(10) DEFAULT 'offline' COMMENT '접속 상태 (online, offline 등)',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '계정 생성 일시',
    FOREIGN KEY (employee_id) REFERENCES employees(employee_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 채팅방 정보를 관리하는 테이블
CREATE TABLE chat_rooms (
    room_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '채팅방 고유 ID',
    room_type VARCHAR(50) NOT NULL COMMENT '채팅방 종류 (direct 또는 group)',
    room_name VARCHAR(255) COMMENT '그룹 채팅방의 이름 (1:1 채팅은 NULL)',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '채팅방 생성 일시'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4; -- InnoDB 엔진 추가

-- 채팅방 참여자 정보를 관리하는 테이블
CREATE TABLE participants (
    room_id INT NOT NULL COMMENT 'chat_rooms 테이블의 ID',
    user_id INT NOT NULL COMMENT 'users 테이블의 ID',
    joined_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '참여 일시',
    PRIMARY KEY (room_id, user_id),
    FOREIGN KEY (room_id) REFERENCES chat_rooms(room_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE -- <<<<<<<<<< 수정됨
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4; -- InnoDB 엔진 추가

-- 모든 메시지를 통합 관리하는 테이블
CREATE TABLE messages (
    message_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '메시지 고유 ID',
    room_id INT NOT NULL COMMENT '메시지가 속한 채팅방 ID',
    sender_id INT NOT NULL COMMENT '메시지를 보낸 사람의 ID',
    content TEXT NOT NULL COMMENT '메시지 내용',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '메시지 작성 일시',
    FOREIGN KEY (room_id) REFERENCES chat_rooms(room_id) ON DELETE CASCADE,
    FOREIGN KEY (sender_id) REFERENCES users(user_id) ON DELETE CASCADE -- <<<<<<<<<< 수정됨
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4; -- InnoDB 엔진 추가

-- 공지사항 정보를 관리하는 테이블
CREATE TABLE notices (
    notice_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '공지사항 고유 ID',
    user_id INT NOT NULL COMMENT '작성자의 ID. users 테이블의 user_id를 참조',
    title VARCHAR(255) NOT NULL COMMENT '공지사항 제목',
    content TEXT NOT NULL COMMENT '공지사항 내용',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '작성 일시',
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '수정 일시',
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE -- <<<<<<<<<< 수정됨
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4; -- InnoDB 엔진 추가
-- 익명 게시물을 관리하는 테이블
CREATE TABLE posts (
    post_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '게시물 고유 ID',
    user_id INT NOT NULL COMMENT '작성자의 ID, users 테이블 참조',
    title VARCHAR(255) NOT NULL COMMENT '게시물 제목',
    content TEXT NOT NULL COMMENT '게시물 내용',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '작성 일시',
    updated_at DATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 익명 게시물에 대한 댓글을 관리하는 테이블
CREATE TABLE comments (
    comment_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '댓글 고유 ID',
    post_id INT NOT NULL COMMENT '어떤 게시물에 대한 댓글인지, posts 테이블 참조',
    user_id INT NOT NULL COMMENT '댓글 작성자의 ID, users 테이블 참조',
    content TEXT NOT NULL COMMENT '댓글 내용',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (post_id) REFERENCES posts(post_id) ON DELETE CASCADE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- 다양한 게시물에 대한 첨부파일을 관리하는 테이블
CREATE TABLE attachments (
    attachment_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '첨부파일 고유 ID',
    related_type VARCHAR(50) NOT NULL COMMENT '어떤 종류의 게시물인지 구분 (예: notice, message)',
    related_id INT NOT NULL COMMENT '연결된 게시물의 고유 ID (예: notice_id, message_id)',
    original_filename VARCHAR(255) NOT NULL COMMENT '사용자가 올린 실제 파일 이름',
    stored_filename VARCHAR(255) NOT NULL UNIQUE COMMENT '서버에 저장되는 고유한 이름',
    file_path VARCHAR(255) NOT NULL COMMENT '파일이 저장된 서버 폴더 경로',
    file_size INT NOT NULL COMMENT '파일 크기 (bytes)',
    mime_type VARCHAR(100) COMMENT '파일 타입 (예: application/pdf, image/png)',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '업로드된 시간'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4; -- InnoDB 엔진 추가

-- 업무 요청 정보를 관리하는 테이블 (due_date 추가됨)
CREATE TABLE work_requests (
    request_id INT AUTO_INCREMENT PRIMARY KEY COMMENT '업무 요청 고유 ID',
    user_id INT NOT NULL COMMENT '요청을 등록한 사용자의 ID, users 테이블 참조',
    title VARCHAR(255) NOT NULL COMMENT '업무 요청 제목',
    content TEXT COMMENT '업무 요청 상세 내용',
    status VARCHAR(50) NOT NULL DEFAULT '접수' COMMENT '처리 상태 (접수, 처리중, 완료, 반려 등)',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP COMMENT '요청 등록 일시',
    due_date DATE NULL COMMENT '업무 마감 기한', -- ✨ 추가된 컬럼
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 업무 담당자(assignee)를 관리하는 중간 테이블
CREATE TABLE work_assignees (
    request_id INT NOT NULL COMMENT '업무 요청 ID, work_requests 테이블 참조',
    assignee_id INT NOT NULL COMMENT '업무를 할당받은 직원의 ID, users 테이블 참조',
    PRIMARY KEY (request_id, assignee_id), -- 두 ID의 조합은 항상 고유해야 함
    FOREIGN KEY (request_id) REFERENCES work_requests(request_id) ON DELETE CASCADE,
    FOREIGN KEY (assignee_id) REFERENCES users(user_id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


CREATE TABLE work_logs (
  log_id int NOT NULL AUTO_INCREMENT,
  user_id int NOT NULL,
  start_time datetime NOT NULL,
  end_time datetime DEFAULT NULL,
  PRIMARY KEY (`log_id`),
  KEY `fk_user_id` (`user_id`),
  CONSTRAINT `fk_user_id` FOREIGN KEY (`user_id`) REFERENCES `users` (`user_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=13 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- related_type과 related_id에 인덱스를 추가하여 조회 성능을 향상시킬 수 있습니다.
CREATE INDEX idx_attachments_related ON attachments(related_type, related_id);