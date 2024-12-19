fn num max [
    arg num x;
    arg num y;

    set result [ cmp_ge x y ];

    if result [ ret x ];
    unless result [ ret y ];
];

log [ max 2 5 ];