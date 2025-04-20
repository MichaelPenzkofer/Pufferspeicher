// ESP32 Einrast-Gehäuseboden mit Führung, Clips, Haken und Kabeldurchführungen

case_l = 100;
case_w = 70;
case_h = 35;
wall = 3;

esp32_l = 51;
esp32_w = 25.4;  // exakt 1 Inch
esp32_z = 3;
esp32_offset = 10;

cable_count = 10;
cable_d = 5;
cable_spacing = 6.5;

hook_w = 4;
hook_d = 2;
hook_h = 5;

// Seitliche Schienen und Drucknasen für ESP32
module esp32_mount(x, y) {
    // seitliche Führungen
    translate([x, y, wall])
        cube([2, esp32_w, 3]); // links
    translate([x + esp32_l - 2, y, wall])
        cube([2, esp32_w, 3]); // rechts

    // Drucknasen
    translate([x + 4, y + 1, wall + 3])
        cube([2, 2, 1]);
    translate([x + esp32_l - 6, y + esp32_w - 3, wall + 3])
        cube([2, 2, 1]);
}

// Einrast-Haken (sichtbar)
module hook(x, y, z) {
    translate([x, y, z])
        hull() {
            cube([hook_w, hook_d, 1]);
            translate([0, 0, hook_h])
                cube([hook_w, hook_d, 1]);
        }
}

difference() {
    // Gehäuse außen
    cube([case_l, case_w, case_h]);

    // Innenhohlraum
    translate([wall, wall, wall])
        cube([case_l - 2*wall, case_w - 2*wall, case_h]);

    // USB-Aussparung
    translate([esp32_offset - 2, case_w - wall, wall + 6])
        cube([10, wall + 1, 6]);

    // Kabeldurchführungen
    for (i = [0:cable_count - 1]) {
        x = (case_l - (cable_count - 1) * cable_spacing)/2 + i * cable_spacing;
        translate([x, 0, 10])
            rotate([90,0,0])
            cylinder(h=wall + 1, d=cable_d, $fn=30);

        // Zugentlastung
        translate([x - 2, wall + 2, 8])
            cube([4, 2, 4]);
    }
}

// ESP32-Führung einbauen
esp32_mount(esp32_offset, (case_w - esp32_w)/2);

// Haken einbauen
hook(15, -hook_d, case_h - hook_h); // vorne links
hook(case_l - 19, -hook_d, case_h - hook_h); // vorne rechts
hook(15, case_w, case_h - hook_h); // hinten links
hook(case_l - 19, case_w, case_h - hook_h); // hinten rechts

