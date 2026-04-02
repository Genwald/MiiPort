struct name {
    char16 name[10];
} [[inline]];

bitfield CoreData {
    hair_type:8;

    height:7;
    mole_type:1;

    build:7;
    hair_flip:1;

    hair_color:7;
    type:1;

    eye_color:7;
    gender:1;

    eyebrow_color:7;
    unused1:1;

    mouth_color:7;
    unused2:1;

    beard_color:7;
    unused3:1;

    glass_color:7;
    unused4:1;

    eye_type:6;
    region_move:2;

    mouth_type:6;
    font_region:2;

    eye_y:5;
    glass_scale:3;

    eyebrow_type:5;
    mustache_type:3;

    nose_type:5;
    beard_type:3;

    nose_y:5;
    mouth_aspect:3;

    mouth_y:5;
    eyebrow_aspect:3;

    mustache_y:5;
    eye_rotate:3;

    glass_y:5;
    eye_aspect:3;

    mole_x:5;
    eye_scale:3;

    mole_y:5;
    unused5:3;

    glass_type:5;
    unused6:3;

    favorite_color:4;
    faceline_type:4;

    faceline_color:4;
    faceline_wrinkle:4;

    faceline_make:4;
    eye_x:4;

    eyebrow_scale:4;
    eyebrow_rotate:4;

    eyebrow_x:4;
    eyebrow_y:4; /* value + 3 = true value */

    nose_scale:4;
    mouth_scale:4;

    mustache_scale:4;
    mole_scale:4;

    name nickname;
};

struct StoreData {
    CoreData coredata;
    u128 create_id;
    u16 crc16;
    u16 crc16_device;
};

StoreData storeData @0 [[inline]];