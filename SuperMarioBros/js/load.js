var loadState = {
    //add a loading.. label on the screen
    preload: function () {
        
        
        game.scale.setGameSize(500, 200);


        //game.load.spritesheet('player', 'assets/player2.png', 20, 20);
        game.load.spritesheet('enemies', 'assets/goomba.png', 16, 16);
        game.load.image('coin', 'assets/coinBlock.png');
        //game.load.image('wallV', 'assets/wallVertical.png');
        //game.load.image('wallH', 'assets/wallHorizontal.png');
        //game.load.image('tileset', 'assets/tileset.png'); 
        game.load.image('tileset', 'assets/Mario.png'); 
        game.load.tilemap('map', 'assets/mapmario.json', null, Phaser.Tilemap.TILED_JSON);
        game.load.tilemap('createmap', 'assets/CreateMap.json', null, Phaser.Tilemap.TILED_JSON);

        //load a new asset that we will use in the menu state
        game.load.image('background', 'assets/background1.png');
       
        game.load.spritesheet('player', 'assets/smallMario.png', 16, 16 );

        //game.load.spritesheet('button1', 'assets/coin.png', 193, 71);
        game.load.spritesheet('button2', 'assets/enemy.png', 193, 71);
        game.load.image('supermario', 'assets/Menu.png')
        game.load.spritesheet('question', 'assets/Mario.png', 16, 16);
        game.load.spritesheet('tube', 'assets/Mario.png', 32, 32);
        game.load.spritesheet('tubebutton', 'assets/tube.png', 16, 16);

        game.load.spritesheet('star', 'assets/star.png', 16, 16);
        game.load.image('fungo', 'assets/fungo.png');
        game.load.image('fungoBig', 'assets/fungoBig.png');
        game.load.spritesheet('flower', 'assets/flower.png', 16, 16);
        game.load.spritesheet('coinBlock', 'assets/coinBlock.png', 16, 16);
        game.load.spritesheet('bigMario', 'assets/big_mario.png', 16, 32);
        game.load.spritesheet('fireMario', 'assets/fireMario.png', 16, 32);
        game.load.spritesheet('pixel', 'assets/Mario.png', 5, 5);
        game.load.spritesheet('fireBall', 'assets/fireBall.png',16,16);
        game.load.spritesheet('koopa', 'assets/koopa.png', 16, 24);
        game.load.spritesheet('splash', 'assets/koopa_squish.png', 16, 16);

        game.load.spritesheet('bandiera', 'assets/b.png', 16 ,16);


    },
    create: function () {
        //go to the menu state
        game.state.start('menu');
    } 
};
