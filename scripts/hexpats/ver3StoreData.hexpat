struct create_id {
    u8 create_id[10];
} [[inline]];

struct name {
    char16 name[10];
} [[inline]];

struct creator_name {
    char16 creator_name[10];
} [[inline]];

bitfield ver3StoreData {
    mii_version:8;
    copyable:1;
    ng_word:1;
    region_move:2;
    font_region:2;
    reserved0:2;
    room_index:4;
    position_in_room:4;
    author_type:4;
    birth_platform:3;
    reserved_1:1;

    u64 author_id;
    create_id create_id;
    u16 reserved_2;

    gender:1;
    birth_month:4;
    birth_day:5;
    favorite_color:4;
    favorite:1;
    padding0:1;

    name name;
    u8 height;
    u8 build;

    localonly:1;
    face_type:4;
    face_color:3;
    face_tex:4;
    face_make:4;

    hair_type:8;
    hair_color:3;
    hair_flip:1;
    padding1:4;

    eye_type:6;
    eye_color:3;
    eye_scale:4;
    eye_aspect:3;

    eye_rotate:5;
    eye_x:4;
    eye_y:5;
    padding2:2;

    eyebrow_type:5;
    eyebrow_color:3;
    eyebrow_scale:4;
    eyebrow_aspect:3;
    padding3:1;

    eyebrow_rotate:5;
    eyebrow_x:4;
    eyebrow_y:5;
    padding4:2;

    nose_type:5;
    nose_scale:4;
    nose_y:5;
    padding5:2;

    mouth_type:6;
    mouth_color:3;
    mouth_scale:4;
    mouth_aspect:3;

    mouth_y:5;
    mustache_type:3;
    padding6:8;

    beard_type:3;
    beard_color:3;
    beard_scale:4;
    beard_y:5;
    padding7:1;

    glass_type:4;
    glass_color:3;
    glass_scale:4;
    glass_y:5;

    mole_type:1;
    mole_scale:4;
    mole_x:5;
    mole_y:5;
    padding8:1;

    creator_name creator_name;
    u16 padding9;
    u16 crc;
};

ver3StoreData ver3StoreData @0 [[inline]];