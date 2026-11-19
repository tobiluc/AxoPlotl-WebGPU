struct ClipBox {
    @align(16) min: vec3<f32>,
    @align(16) max: vec3<f32>,
    @align(16) enabled: i32
};

fn isOutsideClipBox(pos:vec3<f32>, clipBox:ClipBox) -> bool {
    return (clipBox.enabled>0 && ((pos.x < clipBox.min.x)
|| (pos.x > clipBox.max.x) || (pos.y < clipBox.min.y)
|| (pos.y > clipBox.max.y) || (pos.z < clipBox.min.z)
|| (pos.z > clipBox.max.z)));
};

fn isInsideClipBox(pos:vec3<f32>, clipBox:ClipBox) -> bool {
    return !isOutsideClipBox(pos, clipBox);
};

fn clippedPosition() -> vec4<f32> {
    return vec4<f32>(0,0,2,1);
};
