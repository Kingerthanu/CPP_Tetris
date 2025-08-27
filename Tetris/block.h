#pragma once


/*

    Desc: Data Structure Utilized To Manage Our Tetris Grid And Its Blocks
    This->vboOffset Will Help Index And Find Where Data For Specific Blocks
    Reside In The Shared VBO Instance. This->indexOffset Is Likewise, But
    For Its Position In The EBO Instance. To Determine Game Logic And Scoring
    As Well As Collisions, We Use This->occupied To Determine The Occupation
    Status Of A Given Block.

*/
struct Block
{

    unsigned int vboOffset;   // Starting Index In Vertices Array
    unsigned int indexOffset; // Starting Index In Indices Array
    BlockState occupied;

};