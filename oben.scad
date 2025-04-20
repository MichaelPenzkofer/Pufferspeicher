// Deckel mit Rastöffnungen für Einrasthaken

case_l = 100;
case_w = 70;
case_h = 35;
lid_thickness = 3;

hook_w = 4;
hook_d = 2;
hole_clearance = 0.5;

module lid_with_holes() {
    difference() {
        // Deckelplatte
        translate([0, 0, case_h])
            cube([case_l, case_w, lid_thickness]);

        // Rastöffnungen
        for (x = [15, case_l - 19]) {
            // vorne
            translate([x, 0, case_h])
                cube([hook_w, hook_d + hole_clearance, lid_thickness]);
            // hinten
            translate([x, case_w - hook_d - hole_clearance, case_h])
                cube([hook_w, hook_d + hole_clearance, lid_thickness]);
        }
    }
}

lid_with_holes();
