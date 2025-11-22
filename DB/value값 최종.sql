Use yolo_webapp_db;

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE001', '강승진', ' 4팀', 1);
INSERT INTO users (employee_id, username, password, email, phone_number)
VALUES (1, 'kang123', 'kang123', 'ksj000407@themoon.com', '010-4434-3615');

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE002', '송현수', ' 4팀', 0);
INSERT INTO users (employee_id, username, password, email, phone_number)
VALUES (2, 'su123', 'su123', 'su123@themoon.com', '010-9259-1606');

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE003', '김찬송', ' 4팀', 0);
INSERT INTO users (employee_id, username, password, email, phone_number)
VALUES (3, 'song123', 'song123', 'song123@themoon.com', '010-9259-1606');


INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE004', '김주엽', ' 4팀', 0);


INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE005', '이문형', ' 4팀', 0);


INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE006', '강건우', ' 1팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE007', '박지환', ' 1팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE008', '김미진', ' 1팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE009', '천재민', ' 1팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE010', '허영수', ' 1팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE0011', '최상욱', ' 2팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE0012', '차승우', ' 2팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE0013', '박민준', ' 2팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE0014', '이국현', ' 2팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE0015', '황장보', ' 2팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE0016', '성요셉', ' 3팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE0017', '백용', ' 3팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE0018', '정소빈', ' 3팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE0019', '김태은', ' 3팀', 0);

INSERT INTO employees (employee_number, full_name, department, is_admin)
VALUES ('MOBLE0020', '임준용', ' 3팀', 0);

-- yolo_webapp_db 데이터베이스를 사용합니다.
USE yolo_webapp_db;

-- 5. chat_rooms 테이블 데이터 삽입
-- 팀별 단체 채팅방 및 1:1 채팅방 생성
INSERT INTO chat_rooms (room_id, room_type, room_name) VALUES
(1, 'group', '전체 공지방'),
(2, 'group', '1팀'),
(3, 'group', '2팀'),
(4, 'group', '3팀'),
(5, 'group', '4팀'),
(6, 'direct', NULL), -- 강승진(1)과 송현수(2)의 1:1 채팅방
(7, 'direct', NULL); -- 강승진(1)과 김찬송(3)의 1:1 채팅방

-- 6. participants 테이블 데이터 삽입
-- 각 채팅방에 참여자 추가
-- 전체 공지방 (가입된 모든 유저)
INSERT INTO participants (room_id, user_id) VALUES
(1, 1),
(1, 2),
(1, 3);
-- 4팀 채팅방 (4팀 소속 유저)
INSERT INTO participants (room_id, user_id) VALUES
(5, 1),
(5, 2),
(5, 3);
-- 강승진(1)과 송현수(2)의 1:1 채팅방
INSERT INTO participants (room_id, user_id) VALUES
(6, 1),
(6, 2);
-- 강승진(1)과 김찬송(3)의 1:1 채팅방
INSERT INTO participants (room_id, user_id) VALUES
(7, 1),
(7, 3);


-- 7. messages 테이블 데이터 삽입
-- 채팅방별 메시지 예시
INSERT INTO messages (room_id, sender_id, content) VALUES
(1, 1, '전체 공지입니다. 오늘 오후 3시에 주간 회의가 있습니다.'),
(5, 2, '안녕하세요, 4팀 팀원분들. 오늘 점심은 어디로 갈까요?'),
(5, 3, '저는 한식이 좋을 것 같아요.'),
(5, 1, '좋습니다. 그럼 12시에 회사 앞에서 뵙겠습니다.'),
(6, 1, '송현수님, 지난번에 요청드린 자료는 어떻게 되어가나요?'),
(6, 2, '아, 네. 거의 다 마무리되었습니다. 오늘 중으로 전달드리겠습니다.'),
(7, 1, '김찬송님, 주말 잘 보내셨나요?'),
(7, 3, '네, 덕분에 잘 쉬었습니다.');


-- 8. notices 테이블 데이터 삽입
-- 관리자(강승진, user_id: 1)가 작성한 공지사항
INSERT INTO notices (user_id, title, content) VALUES
(1, '2025년 하반기 워크숍 안내', '안녕하세요. 2025년 하반기 워크숍 일정이 확정되어 안내드립니다. 자세한 내용은 첨부파일을 확인해주세요.'),
(1, '사내 보안 강화에 따른 비밀번호 변경 요청', '최근 보안 위협이 증가함에 따라 모든 임직원분들께서는 비밀번호를 변경해주시기 바랍니다.');


-- 9. posts 테이블 데이터 삽입
-- 익명 게시판 게시물
INSERT INTO posts (user_id, title, content) VALUES
(2, '요즘 회사 근처에 맛있는 커피숍 아시는 분?', '매일 같은 곳만 가니 지겹네요. 새로운 곳 추천 부탁드립니다!'),
(3, '다들 퇴근하고 뭐하시나요?', '저는 요즘 운동을 시작했는데, 다른 분들은 어떻게 스트레스를 푸시는지 궁금합니다.');


-- 10. comments 테이블 데이터 삽입
-- 익명 게시판 댓글
INSERT INTO comments (post_id, user_id, content) VALUES
(1, 1, '회사 뒤쪽 골목에 새로 생긴 카페 괜찮더라고요.'),
(1, 3, '저도 거기 가봤는데 원두가 신선해서 좋았어요.'),
(2, 1, '저는 주로 넷플릭스 봅니다 ㅎㅎ');


-- 11. attachments 테이블 데이터 삽입
-- 공지사항 및 메시지에 첨부된 파일 정보 (실제 파일이 아닌 정보만 기록)
INSERT INTO attachments (related_type, related_id, original_filename, stored_filename, file_path, file_size, mime_type) VALUES
('notice', 1, '하반기 워크숍 계획.pdf', 'a1b2c3d4-e5f6-7890-1234-567890abcdef.pdf', '/uploads/notices', 102400, 'application/pdf'),
('message', 6, '업무요청자료.zip', 'z9y8x7w6-v5u4-3210-fedc-ba9876543210.zip', '/uploads/messages', 512000, 'application/zip');


-- 12. work_requests 테이블 데이터 삽입
-- 업무 요청 등록
INSERT INTO work_requests (user_id, title, content, status, due_date) VALUES
(1, '신규 프로젝트 UI/UX 디자인 시안 제작', '9월 신규 프로젝트에 사용될 모바일 앱의 메인 화면과 주요 기능 화면에 대한 디자인 시안을 요청합니다.', '접수', '2025-10-31'),
(2, '2025년 3분기 실적 보고서 작성', '3분기 영업 실적을 정리하여 보고서 형태로 작성해주세요. 다음 주 월요일까지 초안 부탁드립니다.', '처리중', '2025-10-27');


-- 13. work_assignees 테이블 데이터 삽입
-- 업무 요청에 대한 담당자 지정
INSERT INTO work_assignees (request_id, assignee_id) VALUES
(1, 3), -- '신규 프로젝트 UI/UX 디자인' 업무를 김찬송(3)에게 할당
(2, 1), -- '3분기 실적 보고서' 업무를 강승진(1)에게 할당
(2, 2); -- '3분기 실적 보고서' 업무를 송현수(2)에게도 할당 (공동 담당)
