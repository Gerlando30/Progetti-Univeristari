var menuState = {
    create: function () {
        //add a background image
        //var back = game.add.image(0, 0, 'background');
        //back.scale.setTo(1, 0.45);
        game.stage.backgroundColor = '#6495ed';
        this.map = game.add.tilemap('map');
        this.map.addTilesetImage('Mario', 'tileset');
        //create the layer by specifying the name of the Tiled layer
        this.layer = this.map.createLayer('graphics');
        //this.layer.displayHeight = 500;
        //set the world size to match the size of layer
        this.layer.resizeWorld();
        var mario = game.add.image(120, 30, 'supermario');
        mario.scale.setTo(1.5, 1);
        
        var startLabel = game.add.text(game.width/2, game.height-60, 'PRESS A TO START', {
            font: "15px space",
            fill: '#ffffff',
        });
        startLabel.anchor.setTo(0.5, 0.5);

        var createlabel = game.add.text(game.width/2, game.height-40, 'PRESS B TO CREATE YOUR WORLD', {
            font: '15px space',
            fill: '#ffffff'
        });
        createlabel.anchor.setTo(0.5, 0.5);
       
        var nintendo = game.add.text(game.width/2 + 90, game.height-76, '© 1985 NINTENDO', {
            font: '10px space',
            fill: '#ffffff'
        });
        nintendo.anchor.setTo(0.5, 0.5);
        //create a new Pharser keyboard variable:
        //the up arrow key when pressed, call the 'start'
        game.input.keyboard.enabled = true;

        var Bkey = game.input.keyboard.addKey(Phaser.Keyboard.B);
        Bkey.onDown.add(this.createworld, this);

        var Akey = game.input.keyboard.addKey(Phaser.Keyboard.A);
        Akey.onDown.add(this.start, this);

    },
    start: function () {
        //start the actual game
        game.state.start('pre_play');
    },
    createworld: function () {
        game.state.start('editWorld');
    }
};