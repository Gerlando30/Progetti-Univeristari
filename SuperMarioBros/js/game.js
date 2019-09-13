var game = new Phaser.Game(896, 508, Phaser.CANVAS, 'gameDiv');

//define our global variable 
game.global = {
    score: 0,
    life: 3
};
//add all the states
//game.state.add('prova1', stato);
game.state.add('boot', bootState);
game.state.add('load', loadState);
game.state.add('menu', menuState);
game.state.add('editWorld', createState);
game.state.add('play', playState);
game.state.add('pre_play', pre_playState);


//Start the 'boot' state
game.state.start('boot');