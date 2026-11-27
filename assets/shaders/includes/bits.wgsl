alias Word = u32;
alias Word4 = vec4<Word>;

struct BitIndices {
    element_idx: u32,
    word_idx: u32,
    bit_idx: u32
};

fn bit_indices(_idx: u32) -> BitIndices {
    var b:BitIndices;
    b.element_idx = _idx / 128u;
    b.word_idx = (_idx / 32u) % 4u;
    b.bit_idx = _idx % 32u;
    return b;
}

fn read_bit(_word:Word, _bit_idx:u32) -> bool {
    return (_word & (1u << _bit_idx)) != 0u;
}

// Read word with (data)[element_idx][word_idx];
// fn get_bit(_idx: u32) -> bool {
//     let indices = bit_indices(_idx);
//     let word = bit_data[indices.element_idx][indices.word_idx];
//     return read_bit(word, indices.bit_idx);
// }
