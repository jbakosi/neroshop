#include "../include/item.hpp"

////////////////////
neroshop::Item::Item() : id(0)//, quantity(0), price(0.00), weight(1.0), size(std::make_tuple<double, double, double>(0.0, 0.0, 0.0)), discount(0.0), category("unspecified")/* or none */, condition("new") {}// name and desc and empty strings by default
{}
////////////////////
neroshop::Item::Item(unsigned int id) {//: Item() { // for registered items that already have an id
    set_id(id);
}
////////////////////
neroshop::Item::Item(const std::string& name, const std::string& desc, double price, double weight, double length, double width, double height, const std::string& condition, const std::string& product_code) : Item() // quantity is set by cart
{
    register_item(name, desc, price, weight, length, width, height, condition, product_code);
}
////////////////////
neroshop::Item::Item(const std::string& name, const std::string& desc, double price, double weight, const std::tuple<double, double, double>& size, const std::string& condition, const std::string& product_code) : Item() // quantity is set by cart
{
    register_item(name, desc, price, weight, std::get<0>(size), std::get<1>(size), std::get<2>(size), condition, product_code);  
}        
////////////////////
////////////////////
////////////////////
neroshop::Item::~Item() {
#ifdef NEROSHOP_DEBUG0
    std::cout << "item deleted\n";
#endif    
}
////////////////////
void neroshop::Item::register_item(const std::string& name, const std::string& description, double price, double weight, double length, double width, double height, const std::string& condition, const std::string& product_code) {
    ////////////////////////////////
    // sqlite
    ////////////////////////////////
    /*DB::Sqlite3 db("neroshop.db");
    //db.execute("PRAGMA journal_mode = WAL;"); // this may reduce the incidence of SQLITE_BUSY errors (such as database being locked) // https://www.sqlite.org/pragma.html#pragma_journal_mode
    ///////////
    // if item is already registered, then exit function
    if(db.table_exists("item")) {
        int registered_item = db.get_column_integer("item", "id", "product_code = " + DB::Sqlite3::to_sql_string(product_code));
        if(registered_item != 0) {
            neroshop::print("An item with the same product code has already been registered (id will be set regardless)", 2);
            set_id(registered_item);
            db.close(); // DON'T forget to close db right before you return!!
            return; // exit function
        }
    }
    // table item
    if(!db.table_exists("item")) {
	    db.table("item"); // item_id is primary key which will be auto generated
	    db.column("item", "ADD", "name", "TEXT");
	    db.column("item", "ADD", "description", "TEXT"); //db.column("item", "ADD", "quantity", "INTEGER"); // item quantity is managed by cart
	    db.column("item", "ADD", "price", "REAL");
        db.column("item", "ADD", "weight", "REAL");
	    db.column("item", "ADD", "size", "TEXT");//"REAL");//db.column("item", "ADD", "discount", "REAL"); // seller determines the discount//db.column("item", "ADD", "condition", "TEXT"); // seller is able to change the item's condition
	    db.column("item", "ADD", "product_code", "TEXT");
	    db.column("item", "ADD", "category_id", "INTEGER");
	    db.index("idx_product_codes", "item", "product_code"); // item product codes must be unique
	}
	std::string item_size = std::to_string(length) + "x" + std::to_string(width) + "x" + std::to_string(height);//std::to_string(std::get<0>(item.size)) +"x"+ std::to_string(std::get<1>(item.size)) +"x"+ std::to_string(std::get<2>(item.size));
	db.insert("item", "name, description, price, weight, size, product_code", 
	    DB::Sqlite3::to_sql_string(name) + ", " + DB::Sqlite3::to_sql_string(description) + ", " + std::to_string(price) + ", " + std::to_string(weight) + ", " + DB::Sqlite3::to_sql_string(item_size) + ", " + DB::Sqlite3::to_sql_string(product_code) //+ ", " + // + ", " + // + ", " + 
	);
	// save the item id
	unsigned int item_id = db.get_column_integer("item ORDER BY id DESC LIMIT 1", "*");
	set_id(item_id);
    NEROSHOP_TAG_OUT std::cout << "\033[1;36m" << name << " (id: " << get_id() << ") has been registered" << "\033[0m" << std::endl;
    db.close();*/  
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // if an item in registry is branded then a seller cannot sell under that listing unless they sell the same brand
    // to-do: table "brand" and add a "brand_id" column to table item
    // in order for multiple sellers to share the same product detail page, each seller must be selling the exact same product from the exact same manufacturer with the exact same UPC (or whatever product ID code the product uses)
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");

    // if item is already registered, then exit function
    if(DB::Postgres::get_singleton()->table_exists("item")) {
        int registered_item = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM item WHERE product_code = $1", { product_code });
        if(registered_item > 0) {
            neroshop::print("An item with the same product code has already been registered (id will be set regardless)", 2);
            set_id(registered_item);
            //DB::Postgres::get_singleton()->finish(); // DON'T forget to close db right before you return!!
            return; // exit function
        }
    }
    // create table: categories and subcategories
    create_categories_and_subcategories_table();
    //create_table();
    // table item
    if(!DB::Postgres::get_singleton()->table_exists("item")) {
	    DB::Postgres::get_singleton()->create_table("item"); // item_id is primary key which will be auto generated
	    DB::Postgres::get_singleton()->add_column("item", "name", "text");
	    DB::Postgres::get_singleton()->add_column("item", "description", "text"); //db.column("item", "ADD", "quantity", "INTEGER"); // item quantity is managed by cart
	    DB::Postgres::get_singleton()->add_column("item", "price", "numeric(20, 12)"); // numeric is just another word for decimal
        DB::Postgres::get_singleton()->add_column("item", "weight", "real");
	    DB::Postgres::get_singleton()->add_column("item", "size", "text");//"REAL");//db.column("item", "ADD", "discount", "REAL"); // seller determines the discount//db.column("item", "ADD", "condition", "TEXT"); // seller is able to change the item's condition
	    DB::Postgres::get_singleton()->add_column("item", "product_code", "text");
	    //DB::Postgres::get_singleton()->add_column("item", "category_id", "integer REFERENCES categories(id)");
	    //DB::Postgres::get_singleton()->add_column("item", "subcategory_id", "integer REFERENCES categories(id)"); // maybe use an array since item could apply to multiple subcategories
	    // all product codes MUST be unique! I think :O ...
	    DB::Postgres::get_singleton()->create_index("idx_item_product_codes", "item", "product_code"); // item product codes must be unique
	}
	std::string item_size = std::to_string(length) + "x" + std::to_string(width) + "x" + std::to_string(height);//std::to_string(std::get<0>(item.size)) +"x"+ std::to_string(std::get<1>(item.size)) +"x"+ std::to_string(std::get<2>(item.size));
    // add item to records
	DB::Postgres::get_singleton()->execute_params("INSERT INTO item (name, description, price, weight, size, product_code) "
	    "VALUES ($1, $2, $3, $4, $5, $6)"/*, $7)"*/, { name, description, std::to_string(price), std::to_string(weight), item_size, product_code });
	// save the item id (for later use)
	unsigned int item_id = DB::Postgres::get_singleton()->get_last_id("item");
	set_id(item_id);
    NEROSHOP_TAG_OUT std::cout << "\033[1;36m" << name << " (id: " << get_id() << ") has been registered" << "\033[0m" << std::endl;    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////          
#endif    
}
////////////////////
void neroshop::Item::deregister_item() {
    ////delete_all_upload_images();//delete_upload_image();
}
////////////////////
void neroshop::Item::create_table(void) {
    create_categories_and_subcategories_table();
    create_item_table();
}
////////////////////
void neroshop::Item::create_categories_and_subcategories_table(void) {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // TIP: Keep categories simple by merging subcategories in order to make it easier for customers to find specific items
    // create table: categories
    if(!DB::Postgres::get_singleton()->table_exists("categories")) {
         DB::Postgres::get_singleton()->create_table("categories");
         DB::Postgres::get_singleton()->execute("ALTER TABLE categories ADD COLUMN IF NOT EXISTS name text;");
         DB::Postgres::get_singleton()->execute("ALTER TABLE categories ADD COLUMN IF NOT EXISTS alt_names text DEFAULT null;"); // or alternative_names // store in string array anaa ??
         //DB::Postgres::get_singleton()->execute("ALTER TABLE categories ADD COLUMN IF NOT EXISTS desc text DEFAULT null;");
         // category:Pets subcategory:Dog, product:Rottweiler subcategory_parent_id: Dog(id)
         // create table: subcategories
         if(!DB::Postgres::get_singleton()->table_exists("subcategories")) {
             DB::Postgres::get_singleton()->create_table("subcategories");
             DB::Postgres::get_singleton()->execute("ALTER TABLE subcategories ADD COLUMN IF NOT EXISTS name text;");
             DB::Postgres::get_singleton()->execute("ALTER TABLE subcategories ADD COLUMN IF NOT EXISTS category_id integer REFERENCES categories(id);"); // or parent_id
             DB::Postgres::get_singleton()->execute("ALTER TABLE subcategories ADD COLUMN IF NOT EXISTS description text DEFAULT null;");
             // insert categories and their subcategories - these categories and subcategories are only created once
             // category here
             unsigned int category_id = 0; // initial value
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('?', '') RETURNING id;");
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //std::cout << "category: " << category_id << " is created\n";
             //std::cout << "subcategory with parent: " << category_id << " is created\n";
             // subcategories should also be able to have a secondary category (subcategory_id)
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Food', 'Grocery & Gourmet Foods; Produce') RETURNING id;"); // Almost done :) // Food & Beverage in one category? nah
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Canned Foods', $1)", { std::to_string(category_id) }); // sub subcategory
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Condiments', $1)", { std::to_string(category_id) }); // sweet and savory sauces // includes ketchup, mustard, mayonnaise, sour cream, barbecue sauce, compound butter, etc. // A condiment is a preparation that is added to food, typically after cooking, to impart a specific flavor, to enhance the flavor, or to complement the dish 
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Confections', $1)", { std::to_string(category_id) }); // confections or confectionery are sweets like cake, deserts, candies, marshmellows, and other food items that are sweet // sugar confections (aka sweets) = candy, baker/flour confection = cake, chocolate confection = hershey
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Dairy', $1)", { std::to_string(category_id) }); // includes food items like cheese, milk, and yogurt
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Fish & Seafood', $1)", { std::to_string(category_id) }); // includes fish, crab, and all other seafoods
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Food Additives - Sweeteners', $1)", { std::to_string(category_id) }); // includes honey, maple syrup, raw sugar, etc.             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Frozen Foods', $1)", { std::to_string(category_id) }); // sub subcategory
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Fruits', $1)", { std::to_string(category_id) }); // includes berries, melons, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Grains, Breads & Cereals', $1)", { std::to_string(category_id) }); // includes cereal, rice, oatmeal, pasta and traditional bakery like biscuits and bread (whole grains)
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Herbs & Spices', $1)", { std::to_string(category_id) }); //include ginger, turmeric, cinnamon, cocoa powder, parsley, and basil, etc. // herbs are leafy things, like basil, tarragon, thyme, and cilantro; and spices are seeds, either whole or ground, like coriander, cumin, and anise. Leaves vs. seeds is indeed a simple version of the separation. But simple isn't always accurate.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Meat & Poutry', $1)", { std::to_string(category_id) }); //Protein -// All poutry is meat, well since a bird and chicken are meat // fish should have its own subcategory // pescatarians - vegetarians who eat fish and no other flesh or meat
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Nuts, Seeds & Legumes', $1)", { std::to_string(category_id) }); // a nut is a hard-shell containing a seed and a seed is without a shell // legumes are beans
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Fats, Oils, Sugar & Salt', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Organic Foods', $1)", { std::to_string(category_id) }); // sub subcategory
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Processed Foods', $1)", { std::to_string(category_id) }); // sub subcategory
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Proteins - Eggs', $1)", { std::to_string(category_id) }); // eggs are neither dairy (since they don't come from milk, but laid by chickens) nor meat (eggs contain no animal flesh since they are unfertilized)
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Snack Foods', $1)", { std::to_string(category_id) }); // pretzels, slated peanuts, potato chips, popcorn, trail mix, candy, chocolate, baked sweets, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Vegetables', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Vitamins & Dietary Supplements', $1)", { std::to_string(category_id) }); //Nutritional and herbal supplements // includes bee pollen, matcha powder, protein powder, and powdered vitamin or mineral supplements, etc. // Dietary supplements can include a variety of components such as vitamins, minerals, herbs, amino acids, enzymes and other ingredients taken in various forms such as a pill, gel, capsule, gummy, powder, drink or food
             //DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Beverages', '') RETURNING id;"); // DONE! :D
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Alchoholic Beverages - Hard Liquors & Spirits', $1)", { std::to_string(category_id) }); // spirits is another name for liquor (almost), both are distilled beverages and are usually put into these two categories: liquors and spirits // distilled rather than fermented alcoholic beverage
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Alchoholic Beverages - Beer', $1)", { std::to_string(category_id) }); // beer is made from fermented grain
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Alchoholic Beverages - Wine', $1)", { std::to_string(category_id) }); // most wine is produced by fermenting grape juice. However, you can’t just use any type of grape. You need wine grapes
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Coffee', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Juices', $1)", { std::to_string(category_id) }); // lemonade included :} // lemonade could also be a carbonated drink :O // is smoothie a juice? :O
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Milk or milk-based', $1)", { std::to_string(category_id) }); // milkshake, most hot chocolate and some protein shake included :}
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Mixed drinks', $1)", { std::to_string(category_id) }); // cocktail is a mixed drink with a mix of alcohol, fruit juice, soda, etc. :O // what about smoothies? :O
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Soft drinks', $1)", { std::to_string(category_id) }); // soda
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Sports & Energy drinks', $1)", { std::to_string(category_id) }); // sports drinks like gatorade and energy drinks like red bull
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Tea', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Water - Carbonated', $1)", { std::to_string(category_id) }); // same thing as sparkling water
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Water - Non-carbonated', $1)", { std::to_string(category_id) }); // regular drinking water
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Electronics', '') RETURNING id;"); // aka Consumer Electronics or Hardware
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances', $1)", { std::to_string(category_id) }); // Home Appliances
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Dishwashers', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Garbage Disposals & Compactors', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Heating, Cooling & Air Quality Appliances', $1)", { std::to_string(category_id) }); // air conditioners, heaters             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Home Appliance Warranties', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Kitchen Small Appliances', $1)", { std::to_string(category_id) }); // blenders, toasters, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Laundry Appliances', $1)", { std::to_string(category_id) }); // washing machines, dryers, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Microwave Ovens', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Parts & Accessories', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Ranges, Ovens & Cooktops', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Refrigerators, Freezers & Ice Makers', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Outdoor Appliances', $1)", { std::to_string(category_id) }); // grills, smokers
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Appliances - Vacuums & Floor Care Appliances', $1)", { std::to_string(category_id) });
             //DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics - GPS Navigators', $1)", { std::to_string(category_id) });
             //DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics - ', $1)", { std::to_string(category_id) });
             //DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics - Radio and Vehicle audio', $1)", { std::to_string(category_id) });
             //DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Automotive Electronics - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Accessories', $1)", { std::to_string(category_id) }); // accessories for electronics
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('AC Adapters, Chargers, Power strips, & Converters', $1)", { std::to_string(category_id) }); // includes chargers, power outlet (wall) chargers, universal power adapters / Travel Adapters & Converters, power strips, voltage converters, power banks, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cables, Cords, Plugs, and Wires', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cables and Wires - Monitor Cables', $1)", { std::to_string(category_id) }); // usb-c, hdmi, vga, dvi, displayport, rca or component/composite video, thunderbolt 1/2, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cables -', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Calculators', $1)", { std::to_string(category_id) }); // including scientific ones
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Camera & Photo - Camcorders and Digital Cameras', $1)", { std::to_string(category_id) }); // includes digital cameras(DSLR,  Mirrorless,  Compact Cameras / Point & Shoot Cameras, Bridge Cameras, Instant Cameras, Film Cameras, Action Cameras, 360 Degree Cameras, Underwater Cameras / Waterproof Cameras, Medium Format Cameras, SmartPhone Cameras, Rangefinder Cameras, Security Camera, Smartphone Camera, Drone Camera, etc.)
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Camera & Photo - Display Stands', $1)", { std::to_string(category_id) }); // includes digital cameras(DSLR,  Mirrorless,  Compact Cameras / Point & Shoot Cameras, Bridge Cameras, Instant Cameras, Film Cameras, Action Cameras, 360 Degree Cameras, Underwater Cameras / Waterproof Cameras, Medium Format Cameras, SmartPhone Cameras, Rangefinder Cameras, Security Camera, Smartphone Camera, Drone Camera, etc.)             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Camera & Photo - Film Cameras', $1)", { std::to_string(category_id) }); // traditional cameras with film - still digital
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Camera & Photo - Security cameras', $1)", { std::to_string(category_id) }); // includes doorbell cameras, security cameras such as bullet, dome, hidden/covert, infrared, box, outdoor, PTZ, and wireless
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cell Phones - Mobile Phones & Smartphones', $1)", { std::to_string(category_id) }); // aka Mobile phones//DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Phones - Telephones', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Clocks - Digital clocks & Alarm clocks', $1)", { std::to_string(category_id) }); // includes alarm clocks
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Computers - Desktops', $1)", { std::to_string(category_id) }); // aka PC
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Computers - Laptops', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Computers - Laptop Tablet Hybrids', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Computer Parts & Hardware', $1)", { std::to_string(category_id) }); // or Computer Hardware
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Electronic Repair Tools', $1)", { std::to_string(category_id) }); // drills, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Electric Shavers, Razors & Curling Irons', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - 3D Glasses', $1)", { std::to_string(category_id) }); // Entertainment or TV & Theater
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - CD, DVD and Blu-ray players', $1)", { std::to_string(category_id) }); // Blu-ray, is a digital optical disc storage format. It is designed to supersede the DVD format
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - Portable audio players & Boomboxes', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - Radios', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - Remote Controllers', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - Stereo systems', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - TVs', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - TV Receivers', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Entertainment - Video Cassette Recorders (VCR)', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Electronic Musical Instruments', $1)", { std::to_string(category_id) }); // includes Audio Interfaces, Electronic Drums, Digital Synthesizers and Midi Controllers, Microphones, Studio Monitors, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });   
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Hardware - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Headphones, Earphones & Earbuds', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Industrial & Scientific - Electronic Measurement Devices', $1)", { std::to_string(category_id) }); // scales, calipers, etc.  // Industrial & Scientific will have its own main category                     
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             ////DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Lighting - Lamps, Lightbulbs', $1)", { std::to_string(category_id) }); // reading lamps, etc. // put this in Home Tools & Improvements category
             //DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Lawn Mowers', $1)", { std::to_string(category_id) }); // reading lamps, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Headsets', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Microphone', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Mouse', $1)", { std::to_string(category_id) }); // dont forget touchpad and pen input, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Keyboards', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Monitors', $1)", { std::to_string(category_id) }); // screen - led, lcd, ctr, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Power Adapters', $1)", { std::to_string(category_id) }); // laptop power adaptors
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Projectors', $1)", { std::to_string(category_id) }); // screen
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Speakers', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - USB Hubs', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - Webcams', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Peripherals - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Hardware - Ports, Jacks, Slots, Sockets & Connectors', $1)", { std::to_string(category_id) }); // includes audio jacks, usb ports, microSD card reader, etc. // all plugs and ports are connectors
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices - CB & Two-way Radio', $1)", { std::to_string(category_id) }); // walkie-talkies             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices - Media Players', $1)", { std::to_string(category_id) }); // portable mp3 players, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Portable Media Devices - ', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Printers - 3D Printers', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Printers - Inkjet Printers', $1)", { std::to_string(category_id) }); // single function printers only print while multi-function printers can print, scan, copy, and fax
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Printers - Laser Printers', $1)", { std::to_string(category_id) });             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Printers, Scanners, Copiers, & Fax machines', $1)", { std::to_string(category_id) }); // single function printers only print while multi-function printers can print, scan, copy, and fax
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Ink and toner cartridges', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Safe box', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media', $1)", { std::to_string(category_id) }); // includes hard disk drives (hdd), solid state drives (ssd), m.2 ssd, network attached storage (nas), etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - Cassette Tapes', $1)", { std::to_string(category_id) }); // small ones for audio, big ones for video
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - CDs/DVDs/Blu-rays', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - Floppy Disks', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - Game Discs & ROM Catridges', $1)", { std::to_string(category_id) }); // aka game cartridge, cart, or card
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - Hard disk drives (HDD), Solid state drives (SSD)', $1)", { std::to_string(category_id) }); // includes hdd, sdd, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - Memory Cards', $1)", { std::to_string(category_id) }); // includes sd cards, card readers
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Storage Media - USB Flash Drives', $1)", { std::to_string(category_id) }); // flash memory
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Tablets', $1)", { std::to_string(category_id) }); // includes ipad, android tablets, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Tablets - Graphics and Drawing Tablets', $1)", { std::to_string(category_id) }); // includes pen tablets (without a screen) and graphics tablets like wacom, etc. // https://www.autopano.net/types-of-drawing-tablets/
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Toys & Games', $1)", { std::to_string(category_id) }); // toys and games should have a category of its own
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Toys & Games - Electronic Toys', $1)", { std::to_string(category_id) });// includes electric car toys, remote control cars, drones, robots, etc. // toys and games should have a category of its own
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Video Game Consoles, Games & Accessories', $1)", { std::to_string(category_id) });  // includes video game controllers, remote controllers, headsets, virtual reality headsets (PSVR), covers, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology', $1)", { std::to_string(category_id) });             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Body Mounted Cameras', $1)", { std::to_string(category_id) });      
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Clips, Arm & Wristbands', $1)", { std::to_string(category_id) });      
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Glasses', $1)", { std::to_string(category_id) });             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Rings', $1)", { std::to_string(category_id) });             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Smartwatches', $1)", { std::to_string(category_id) });             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wearable Technology - Virtual Reality Gear & Headsets', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Digital Goods', '') RETURNING id;"); // DONE :D! // SELECT * FROM subcategories WHERE category_id = $1;--ORDER BY name;
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Codes - Cheat codes and hacks', $1)", { std::to_string(category_id) }); // replace "Codes" with "Information" anaa? :O // includes hacked information and leaks
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Codes - Discount and coupons', $1)", { std::to_string(category_id) }); // discounts, coupons/vouchers, promo codes etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Codes - Gift cards', $1)", { std::to_string(category_id) }); // gift certificates included
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Codes - Membership and subscription cards', $1)", { std::to_string(category_id) }); // PSN Membership card, PS+ subscription card, Xbox Live cards, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Codes - Prepaid cards', $1)", { std::to_string(category_id) }); // A prepaid card is not linked to a bank checking account or to a credit union share draft account. Instead, you are spending money you placed in the prepaid card account in advance. This is sometimes called “loading money onto the card”.// For hacked credit cards, this subcategory can be used >:D JK!
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Digital Art', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Digital Currencies - Cryptocurrencies', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Digital Currencies - Fiat', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('eBooks', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Freelancing', $1)", { std::to_string(category_id) }); // sell your personal services online in a gig-economy and work from home // freelancing is different and better than remote work cuz you are your own boss // Gigs are also known as “freelancing” and “independent contracting” jobs
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Information - Online accounts', $1)", { std::to_string(category_id) }); // includes online accounts such as video game accounts, netflix accounts, etc.        
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Media - Audio', $1)", { std::to_string(category_id) }); // sound, video, text, etc. // Multimedia is the combined use of sound, video, and text to present an idea
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Media - Documents', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Media - Photos', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Media - Videos', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Software', $1)", { std::to_string(category_id) }); // general software
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Video Games', $1)", { std::to_string(category_id) }); // video games are softwares with their own category :O
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Virtual Goods - In-Game content', $1)", { std::to_string(category_id) });  // video game/virtual avatars, items, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Virtual Goods - NFTs', $1)", { std::to_string(category_id) }); // NFTs can be more than just art ranging from Fashion and wearables, DeFi, Events and ticketing, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Websites and Domain names', $1)", { std::to_string(category_id) });
             //DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Books', '') RETURNING id;");
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Children''s Books', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Comics & Manga', $1)", { std::to_string(category_id) }); // include Manga?
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cookbooks', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Journals', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Magazines', $1)", { std::to_string(category_id) });             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Newspapers', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Poetry', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Textbooks', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Novels', $1)", { std::to_string(category_id) });             
             //DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Apparel', 'Clothing, Shoes and Accessories; Fashion') RETURNING id;"); // Wearables // Almost DONE :)!
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Armwear', $1)", { std::to_string(category_id) }); // gloves, bracelets, sleeves, armband, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Baby & Toddler', $1)", { std::to_string(category_id) }); // diapers, bibs, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Bags, Handbags, Backpacks, Wallets, Purses & Pouches', $1)", { std::to_string(category_id) }); // backpacks, handbags, purses, totes, laptop bags, duffel bags, fanny packs/belt bags, briefcases, etc. // https://shilpaahuja.com/types-of-bags/  // Carry Goods or Carry Access.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Bottomwear', $1)", { std::to_string(category_id) }); // pants/trousers/jeans, shorts, skirts, miniskirts, leggings, breeches, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cookwear - Aprons & Bibs', $1)", { std::to_string(category_id) }); // kappōgis, pinafores, etc.             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Costumes & Cosplay', $1)", { std::to_string(category_id) }); // costumes, reanactment and theater, and masks too
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Dresses & Gowns', $1)", { std::to_string(category_id) }); // dresses, blouses, gowns
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Eyewear', $1)", { std::to_string(category_id) }); // eyeglasses, googles, sunglasses / shades, monocles, eyepatches, contact lenses, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Footwear', $1)", { std::to_string(category_id) }); // socks, boots, shoes / sneakers, slippers, sandals, heels, flats, crocs             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Handwear - Gloves, Mittens', $1)", { std::to_string(category_id) }); // gloves, mittens, boxing gloves, rubber gloves, knuckle dusters, etc.          
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Headwear', $1)", { std::to_string(category_id) }); // hats, caps, bandanas, headbands, crowns, wigs, kerchiefs, helmets, hijabs, hoods, headscarfs, turbans, hair nets, face masks, theatre masks, gas masks, sports mask for hockey, masquerade ball masks, ritual masks, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Hosiery - Socks & stockings, Pantyhose, Tights, etc.', $1)", { std::to_string(category_id) }); // socks & stockings, knee-highs, pantyhose, tights, fishnets + garters, leg warmers, yoga pants, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Kids'' Wear', $1)", { std::to_string(category_id) }); // or Children''s
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Knee clothing', $1)", { std::to_string(category_id) }); // knee-pads, etc.             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Men''s Wear', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Neckwear', $1)", { std::to_string(category_id) }); // necklaces, chokers / collars, scarfs, neckties
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Outerwear', $1)", { std::to_string(category_id) }); // coats, sweaters / hoodies, and jackets
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Pets - Pets'' clothing', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Robes and cloaks', $1)", { std::to_string(category_id) }); // sleeping gowns, bath robes, ponchos, academic dress, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Scarfs, Shawls, Wraps & Pashimas', $1)", { std::to_string(category_id) }); // scarfs, shawls and wraps, pashimas
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Sleepwear & Pajamas', $1)", { std::to_string(category_id) }); // pyjamas, pyjama pants, night gowns, sleep shirts, etc. // Nightwear
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Sportswear', $1)", { std::to_string(category_id) }); // jerseys, swimsuits, tracksuits, bathing suits, wet suits, ski wear, motorcycle gear, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Suits & One-piece suits', $1)", { std::to_string(category_id) }); // one-piece suits include: Sling swimsuit, jumpsuits, flight/space suits, ski suits, chemturion, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Textiles', $1)", { std::to_string(category_id) }); // cloth pieces, fabrics:knitted, woven, and non-woven,  silk, cotton, leather, etc. // https://www.masterclass.com/articles/28-types-of-fabrics-and-their-uses#28-different-types-of-fabric  // 
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Topwear - Shirts & Tops', $1)", { std::to_string(category_id) }); // tees, tank-tops/singlets, vests, coats, sweaters, and jackets, cardigans
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Topwear', $1)", { std::to_string(category_id) }); // tees, tank-tops/singlets, vests, coats, sweaters, and jackets, cardigans             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Traditional Wear', $1)", { std::to_string(category_id) }); // kimonos, kilts, sari(s), hwarots, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Underwear', $1)", { std::to_string(category_id) }); // slips, panties and boxers, bras, other undergarments
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Uniforms & Work Clothing', $1)", { std::to_string(category_id) }); // school uniforms, police uniforms, etc.
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Unisex', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Waistwear - Belts & Suspenders', $1)", { std::to_string(category_id) }); // belts, sashes like obi, karate belts, etc.             
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Watches, Rings & Jewelry', $1)", { std::to_string(category_id) }); // accessory: rings, earrings, watches, bracelets, chains, necklaces, pendants, charms, jewelry clasps / hooks, 
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Wedding & Formal', $1)", { std::to_string(category_id) }); // wedding gowns, dress shirts/men's suits/sweater vest/waistcoat/lounge jackets(suit coats) // formal attire
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Women''s Wear', $1)", { std::to_string(category_id) });
             //DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Pets', 'Domesticated Animals') RETURNING id;");
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Dogs', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Cats', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Birds', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Fishes', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Reptiles', $1)", { std::to_string(category_id) });
             //DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('Pet Supplies', $1)", { std::to_string(category_id) });
             //DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Cosmetics', 'Beauty & Personal Care') RETURNING id;"); // skin and hair care
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             /*//---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('?', '') RETURNING id;");
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('?', '') RETURNING id;");
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('?', 'Musical Instruments') RETURNING id;");
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('?', '') RETURNING id;");
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------             
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('?', '') RETURNING id;");
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('?', '') RETURNING id;");
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------             
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('?', '') RETURNING id;"); // Sports & Outdoor Goods => Camping & Hiking, Garden (pots, plants, gardening tools), Cookouts & Outdoor Furniture (patios, etc.), Recreation (basketball hoop, etc.), Fishing & Hunting
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('Everything Else', 'Miscellaneous') RETURNING id;");
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------             */
         }
    }
#endif    
}
////////////////////
void neroshop::Item::create_item_table(void) {
}
////////////////////
// DELETE THIS FUNCTION AS WE NO LONGER HAVE A USE FOR IT
void neroshop::Item::register_item(const neroshop::Item& item) { 
    // if item is already registered, then exit function
    /*if(item.is_registered()) {std::cout << "\033[0;93m" << "Item " << item.name << " has already been registered" << "\033[0m" << std::endl;return;}
    DB::Sqlite3 db("neroshop.db");
    //db.execute("PRAGMA journal_mode = WAL;"); // this may reduce the incidence of SQLITE_BUSY errors (such as database being locked) // https://www.sqlite.org/pragma.html#pragma_journal_mode
    ///////////
    int reg_item = db.get_column_integer("item", "id", "product_code = " + DB::Sqlite3::to_sql_string(item.product_code));
    if(reg_item != 0) {
        neroshop::print("Item with the same product code is registered", 1);
        const_cast<neroshop::Item&>(item).set_id(reg_item);
        NEROSHOP_TAG_OUT std::cout << "item id set to: " << reg_item << std::endl;
        return; // exit function
    }*/
/*
             // category here
             category_id = DB::Postgres::get_singleton()->get_integer("INSERT INTO categories (name, alt_names) VALUES ('?', '') RETURNING id;");
             // subcategories here
             DB::Postgres::get_singleton()->execute_params("INSERT INTO subcategories (name, category_id) VALUES ('?', $1)", { std::to_string(category_id) });
             //---------------------------
             */
    ///////////
    /*if(!db.table_exists("categories")) {
	    db.table("categories");
	    db.column("categories", "ADD", "category_name", "TEXT");
	    // subcategories
	    if(!db.table_exists("subcategories")) {
	        db.table("subcategories");
	        db.column("subcategories", "ADD", "subcategory_name", "TEXT");
	        int category_id = 0;
	        // these categories and subcategories are only created once
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Appliances")); // Furniture and Appliances
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "category_id, subcategory_name", std::to_string(category_id) + ", " + DB::Sqlite3::to_sql_string(""));

	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Arts, Crafts, & Sewing")); // paints, ceramics, etc
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "category_id, subcategory_name", std::to_string(category_id) + ", " + DB::Sqlite3::to_sql_string(""));	        	        	        	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Automotive Parts & Accessories")); // Automotive & Powersports, Bikes
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "category_id, subcategory_name", std::to_string(category_id) + ", " + DB::Sqlite3::to_sql_string(""));

	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Baby")); // excluding apparel
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "category_id, subcategory_name", std::to_string(category_id) + ", " + DB::Sqlite3::to_sql_string(""));
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("CDs & Vinyl"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Camera & Photo"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Cell Phones & Accessories"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	
	               	        	        	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Clothing, Shoes and Jewelry")); // Apparel // https://sewguide.com/types-of-clothes/
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("Aprons") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("Bras") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("Coats, Jackets, and Cardigans") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("Dresses") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("Footwear") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("Shorts, Pants and Jeans") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("Skirts") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("Socks") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("Sweaters, Jackets") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("T-shirts, Tees, Tops") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("Traditional clothing") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("Underwear") + ", " + std::to_string(category_id));
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Collectible Coins")); // Collectibles & Fine Art, Art - Fine, Art - Home Decor// Currency, Coins, Cash Equivalents, and Gift Cards
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));        
	 
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Entertainment Collectibles"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Fine Art"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	
	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Handmade"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Health & Personal Care")); // Health, Household & Baby Care
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	        	        	        	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Home & Kitchen")); // furniture, coffee makers, etc.
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Home & Garden")); // Garden & Outdoor
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Independent Design"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	 
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Industrial & Scientific"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Luggage & Travel Gear"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Major Appliances"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	        	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Movies & TV"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Music and DVD"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Musical Instruments"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	        	        	        	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Office Products"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Outdoors"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));

	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Software")); // Apps & Games
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Sports")); // Sports & Outdoors
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));	        	        	        	        
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Sports Collectibles"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Tools & Home Improvement"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	 
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Toys & Games")); // Toys, Hobbies, DIY
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	        
	        db.insert("categories", "category_name", DB::Sqlite3::to_sql_string("Video, DVD & Blu-ray"));
	        category_id = db.get_column_integer("categories ORDER BY id DESC LIMIT 1", "*"); // last inserted category
	        //db.insert("subcategories", "subcategory_name, category_id", DB::Sqlite3::to_sql_string("") + ", " + std::to_string(category_id));
	    }
	}*/
    ///////////
    /*if(!db.table_exists("item")) {
	    db.table("item"); // item_id is primary key which will be auto generated
	    db.column("item", "ADD", "name", "TEXT");
	    db.column("item", "ADD", "description", "TEXT"); //db.column("item", "ADD", "quantity", "INTEGER"); // item quantity is managed by cart // seller determines how much is in stock
	    db.column("item", "ADD", "price", "REAL");
        db.column("item", "ADD", "weight", "REAL");
	    db.column("item", "ADD", "size", "TEXT");//"REAL");
	    db.column("item", "ADD", "discount", "REAL");
	    db.column("item", "ADD", "condition", "TEXT"); // seller determines the item's condition
	    db.column("item", "ADD", "product_code", "TEXT");
	    db.column("item", "ADD", "category_id", "INTEGER");
	    db.index("idx_product_codes", "item", "product_code"); // item product codes must be unique
	}
	std::string item_size = std::to_string(std::get<0>(item.size)) +"x"+ std::to_string(std::get<1>(item.size)) +"x"+ std::to_string(std::get<2>(item.size));
	db.insert("item", "name, description, price, weight, size, discount, condition, product_code", 
	    DB::Sqlite3::to_sql_string(item.name)         + ", " + 
	    DB::Sqlite3::to_sql_string(item.description)  + ", " + //std::to_string(item.get_quantity()) + ", " + 
	    std::to_string(item.price)           + ", " + // original / list price
	    std::to_string(item.weight)          + ", " + 
	    DB::Sqlite3::to_sql_string(item_size)         + ", " + 
	    std::to_string(item.discount)        + ", " + 
	    DB::Sqlite3::to_sql_string(item.condition)    + ", " + 
	    DB::Sqlite3::to_sql_string(item.product_code) //+ ", " + 
	    // + ", " + 
	    // + ", " + 
	);
	// save the item id
	unsigned int item_id = db.get_column_integer("item ORDER BY id DESC LIMIT 1", "*");
	const_cast<neroshop::Item&>(item).set_id(item_id);
    ///////////
    //const_cast<neroshop::Item&>(item).show_info();
    NEROSHOP_TAG_OUT std::cout << "\033[1;36m" << item.name << " (id: " << item.id << ") has been registered" << "\033[0m" << std::endl;
    ///////////
    db.close();*/
}
////////////////////
// seller should have the option to upload videos too (when videos are supported)
void neroshop::Item::upload(const std::string& filename) { 
#if defined(NEROSHOP_USE_POSTGRESQL)
/*#ifdef __gnu_linux__
    char file[1024];
    FILE * f = popen("zenity --file-selection", "r");
    fgets(file, 1024, f);
    int pid = pclose(f);//if(f) pclose(f);
#endif
    //Process process("zenity", "--file-selection");
    std::string filename(file);*/
    ///////////////////////////
    if(this->id == 0) {
        neroshop::print("This item does not exist in the database", 1);
        return;
    }
    ///////////////////////////
    // create table
    if(!DB::Postgres::get_singleton()->table_exists("images")) {
        DB::Postgres::get_singleton()->create_table("images");
        DB::Postgres::get_singleton()->add_column("images", "item_id", "integer REFERENCES item(id)");
    	DB::Postgres::get_singleton()->add_column("images", "name", "text ARRAY");// include ext ////DB::Postgres::get_singleton()->add_column("images", "file", "text");
    	// bytea is limited to 1GB and OID is limited to 2GB
    	// The oid type is currently implemented as an unsigned four-byte integer: https://www.postgresql.org/docs/current/datatype-oid.html
    	DB::Postgres::get_singleton()->execute("ALTER TABLE images ADD COLUMN IF NOT EXISTS data oid ARRAY;");//"bytea");//"blob");//"lo");
    }
    //////////////////////////
    std::string image_file = filename;
    std::string image_name = image_file.substr(image_file.find_last_of("\\/") + 1);// get filename from path
    image_name = image_name.substr(0, image_name.find_last_of(".")); // remove ext
    std::string image_ext = image_file.substr(image_file.find_last_of(".") + 1);
    //std::cout << "image name: " << image_name << std::endl;
    //std::cout << "image file: " << image_file << std::endl;
    //std::cout << "image extension: " << image_ext << std::endl;    
    ///////////////////////////
    /*// bytea
    // insert the image elephant.png
    DB::Postgres::get_singleton()->execute_params("INSERT INTO images(name, data) VALUES($1, bytea_import($2))", { image_name, image_file }); // pg_read_binary_file($2) // pg_read_file($2)::bytea
    // select data
    std::cout << DB::Postgres::get_singleton()->get_text("SELECT name, data FROM images;") << std::endl;*/ //;//DB::Postgres::get_singleton()->execute_params("", {});
    ///////////////////////////
    // https://www.postgresql.org/docs/current/lo-interfaces.html
    DB::Postgres::get_singleton()->execute("BEGIN;");
    // Before we start anything, lets make sure number of images do not exceed MAX_IMAGES_PER_ITEM
    int image_count = DB::Postgres::get_singleton()->get_integer_params("SELECT cardinality(data) FROM images WHERE item_id = $1", { std::to_string(this->id) });//("SELECT COUNT(*) FROM images WHERE item_id = $1", { std::to_string(this->id) });//std::cout << "image_count: " << image_count << std::endl; // temp
    int MAX_IMAGES_PER_ITEM = 1;//6;// for now lets keep the max_images to 1. Will increase later on
    if(image_count >= MAX_IMAGES_PER_ITEM) {
        neroshop::print("MAX_IMAGES_PER_ITEM has been exceeded! Items can only have up to " + std::to_string(MAX_IMAGES_PER_ITEM) + " image(s)", 1);
        DB::Postgres::get_singleton()->execute("ROLLBACK;"); // abort transaction
        return; // exit function
    }
    // NOTE: Client applications cannot use these functions while a libpq connection is in pipeline mode // https://www.postgresql.org/docs/current/libpq-pipeline-mode.html
    // create a new large object
    Oid image_oid = lo_creat(DB::Postgres::get_singleton()->get_handle(), INV_READ|INV_WRITE);
    if(image_oid == InvalidOid) { std::cout << NEROSHOP_TAG POSTGRESQL_TAG_ERROR "failed to create OID " << image_oid << "\033[0m" << std::endl; DB::Postgres::get_singleton()->execute("ROLLBACK;"); return; }
    std::cout << image_oid << std::endl; // 81944, 81963 // make sure oid is not InvalidOid (zero)
    ///////////////////////////
    // open the existing large object descriptor (for writing)
    int image_fd = lo_open(DB::Postgres::get_singleton()->get_handle(), image_oid, INV_WRITE); // write only
    if(image_fd == -1) {
        std::cout << NEROSHOP_TAG POSTGRESQL_TAG_ERROR "failed to open OID " << image_oid << " for writing\033[0m" << std::endl;
        DB::Postgres::get_singleton()->execute("ROLLBACK;"); // abort transaction
        return; // exit function
    }    
    //////////////////////////
    //static char lo_content[] = "Lorem ipsum dolor sit amet, fabulas conclusionemque ius ad.";//std::cout << "lo_content size: " << sizeof(lo_content) << std::endl;//60
    std::ifstream image_file_r(image_file, std::ios::binary); // std::ios::binary is the same as std::ifstream::binary
    if(!image_file_r.good()) {
        std::cout << NEROSHOP_TAG "failed to load " << image_file << std::endl; DB::Postgres::get_singleton()->execute("ROLLBACK;"); return;
    }
    image_file_r.seekg(0, std::ios::end); // std::ios::end is the same as image_file_r.end
    size_t file_size = static_cast<int>(image_file_r.tellg()); // .tellg() is a std::streampos
    image_file_r.seekg(0); // image_file_r.seekg(0, image_file_r.beg);
    std::vector<unsigned char> buffer(file_size); // or unsigned char buffer[file_size];// or unsigned char * buffer = new unsigned char[file_size];
    if(!image_file_r.read(reinterpret_cast<char *>(&buffer[0]), file_size)) {// read data as a block
        std::cout << NEROSHOP_TAG "error: only " << image_file_r.gcount() << " could be read";
        DB::Postgres::get_singleton()->execute("ROLLBACK;"); // abort transaction
        return; // exit function
    }    
    ////for(auto content : buffer) std::cout << content << std::endl; // temp
    image_file_r.close();
    ///////////////////////////
    // write image data to the large object descriptor
    if(lo_write(DB::Postgres::get_singleton()->get_handle(), image_fd, reinterpret_cast<const char *>(buffer.data())/*lo_content*/, file_size/*sizeof(lo_content)*/) == -1) {//image_write, (image.get_width() * image.get_height() * image.get_channel())) == -1) {
        std::cout << NEROSHOP_TAG POSTGRESQL_TAG_ERROR "failed to write to large object file descriptor " << image_fd << "\033[0m" << std::endl;
    }
    // close large object descriptor
    if(lo_close(DB::Postgres::get_singleton()->get_handle(), image_fd) == -1) {
        std::cout << NEROSHOP_TAG POSTGRESQL_TAG_ERROR "failed to close large object file descriptor " << image_fd << "\033[0m" << std::endl;
        DB::Postgres::get_singleton()->execute("ROLLBACK;"); // abort transaction
        return; // exit function if file fails to both write and close
    }
    // buffer.clear(); // use "delete [] buffer;" only if char * was allocated with new // buffer contains the entire file (its content)
    ///////////////////////////
    // multiple product images can possibly have the same name so we have to make the names a bit unique perhaps by appending the oid to the name
    // append this before the .extension
    image_name = image_name + ("_" + std::to_string(image_oid)) + ("." + image_ext);//std::cout << "UPDATED IMAGE NAME(WITH EXT INCLUDED): " << image_name << std::endl;
    // if image_id is already in table image, update its contents
    std::string item_id_found = DB::Postgres::get_singleton()->get_text_params("SELECT exists (SELECT item_id FROM images WHERE item_id = $1 LIMIT 1);", { std::to_string(this->id) });
    bool item_id_found_in_table_image = (item_id_found == "t") ? true : false;
    if(item_id_found_in_table_image) { 
        DB::Postgres::get_singleton()->execute_params("UPDATE images SET name = array_append(name, $1::text) WHERE item_id = $2", { image_name, std::to_string(this->id) });
        DB::Postgres::get_singleton()->execute_params("UPDATE images SET data = array_append(data, $1::oid) WHERE item_id = $2", { std::to_string(image_oid), std::to_string(this->id) });//DB::Postgres::get_singleton()->execute_params("UPDATE images SET data = data || '{$1::oid}' WHERE item_id = $2", { std::to_string(image_oid), std::to_string(this->id) });
        neroshop::print("added image upload", 3);
    }
    // check if item_id is already in table image. If not, insert item_id into table image
    if(!item_id_found_in_table_image) { 
        DB::Postgres::get_singleton()->execute_params("INSERT INTO images(item_id, name, data) VALUES($1, $2::text[], $3::oid[])", { std::to_string(this->id), "{" + image_name + "}", "{" + std::to_string(image_oid) + "}" }); // or $3::oid[] with "{" + std::to_string(image_oid) + "}"////DB::Postgres::get_singleton()->execute_params("INSERT INTO images(item_id, name, data) VALUES($1, $2, $3::oid)", { std::to_string(this->id), image_name, std::to_string(image_oid) });
        neroshop::print("inserted new image data", 2);
    }        
    ///////////////////////////
    // check to see if image data is correct by exporting large object (optional)
    // 
    /*if(lo_export(DB::Postgres::get_singleton()->get_handle(), image_oid, ("/tmp/" + image_name + ".png").c_str()) == -1) { // Returns 1 on success, -1 on failure
        std::cout << POSTGRESQL_TAG_ERROR "failed to import file to " << ("/tmp/" + image_name + ".png").c_str() << "\033[0m" << std::endl;
    }*/
    // import the image file
    ////std::cout << lo_import_with_oid(DB::Postgres::get_singleton()->get_handle(), image_file.c_str(), image_oid) << std::endl; // 0 InvalidOid (zero)//lo_import(DB::Postgres::get_singleton()->get_handle(), image_file.c_str()); // returns InvalidOid (zero)    
    ///////////////////////////
    DB::Postgres::get_singleton()->execute("COMMIT;");    
    //DB::Postgres::get_singleton()->finish();
#endif    
}    
////////////////////
// the first image of an item will be used as the thumbnail
Image * neroshop::Item::get_upload_image(int index) const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // begin transaction
    DB::Postgres::get_singleton()->execute("BEGIN;");
    // open the existing large object (for reading)
    Oid image_oid = DB::Postgres::get_singleton()->get_integer_params("SELECT data[$1] FROM images WHERE item_id = $2;", { std::to_string(index), std::to_string(this->id) });//("SELECT data FROM images WHERE item_id = $1;", { std::to_string(this->id) });//std::cout << image_oid << std::endl;
    if(image_oid == InvalidOid) { std::cout << NEROSHOP_TAG POSTGRESQL_TAG_ERROR "OID not found for this particular image (No image(s) uploaded for this item) " << "\033[0m" << std::endl; DB::Postgres::get_singleton()->execute("ROLLBACK;"); return nullptr; } // ABORT; is the same as ROLLBACK;
    // get image name (with extension included)
    std::string image_name = DB::Postgres::get_singleton()->get_text_params("SELECT name[$1] FROM images WHERE item_id = $2;", { std::to_string(index), std::to_string(this->id) });//std::cout << image_name << std::endl;
    ///////////////////////////
    ////std::cout << lo_import_with_oid(DB::Postgres::get_singleton()->get_handle(), "/tmp/random.png", image_oid) << std::endl;
    // NEROSHOP_PATH is defined in "neroshop/src/debug.hpp"
    // get neroshop directory and place image in cache folder within the neroshop directory instead
    if(!File::is_directory(NEROSHOP_PATH)) { neroshop::print("directory \"" + std::string(NEROSHOP_PATH) + "\" not found", 1); DB::Postgres::get_singleton()->execute("ROLLBACK;"); return nullptr; }
    std::string neroshop_cache_path = NEROSHOP_PATH + "/cache";
    if(!File::is_directory(neroshop_cache_path)) { 
        neroshop::print("directory \"" + neroshop_cache_path + "\" does not exist, but I will create the path for you (^_^)", 2);
        if(!File::make_dir(neroshop_cache_path)) {
            neroshop::print("I could not create folder \"" + neroshop_cache_path + "\". Forgive me senpai (ᵕ人ᵕ)! ...", 1);
            DB::Postgres::get_singleton()->execute("ROLLBACK;"); // abort transaction
            return nullptr;
        }
        neroshop::print("\033[1;97;49mcreated path \"" + neroshop_cache_path + "\""); // bold bright white
    }
    // check if image already exists in cache so that we do not export the same image more than once
    std::string image_export_path = neroshop_cache_path + "/" + image_name;//std::cout << "image_export_path: " << image_export_path << std::endl;
    if(File::exists(image_export_path)) {
        ////neroshop::print("file \"" + image_export_path + "\" already exists so I will skip it (.‿.) ...");
        Image * image = new Image(image_export_path); // can't use shared_ptr here since it dies at the end of the scope :/
        DB::Postgres::get_singleton()->execute("ROLLBACK;"); // abort transaction
        return image; // exit function after loading the pre-existing image file
    }
    // export large object to an external file    
    if(lo_export(DB::Postgres::get_singleton()->get_handle(), image_oid, image_export_path.c_str()) == -1) { // Returns 1 on success, -1 on failure
        std::cout << NEROSHOP_TAG POSTGRESQL_TAG_ERROR "failed to export file to " << image_export_path.c_str() << "\033[0m" << std::endl;
        DB::Postgres::get_singleton()->execute("ROLLBACK;"); // abort transaction
        return nullptr;
    }
    neroshop::print("exported \"" + image_name + "\" to \"" + neroshop_cache_path + "\"", 3);
    ///////////////////////////
    // end transaction
    DB::Postgres::get_singleton()->execute("COMMIT;");
    ///////////////////////////
    // create an image object, load the image then return the image object
    Image * image = new Image(image_export_path); // can't use shared_ptr here since it dies at the end of the scope :/
    return image;//->get_data();
    ///////////////////////////
#endif    
    return nullptr;
}
////////////////////
void neroshop::Item::delete_upload_image(int index) {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // begin transaction (required when dealing with large objects)
    DB::Postgres::get_singleton()->execute("BEGIN;");
    // get image oid (large object)
    Oid image_oid = DB::Postgres::get_singleton()->get_integer_params("SELECT data[$1] FROM images WHERE item_id = $2;", { std::to_string(index), std::to_string(this->id) });//std::cout << image_oid << std::endl;
    if(image_oid == InvalidOid) { DB::Postgres::get_singleton()->execute("ROLLBACK;"); return; } // ABORT; is the same as ROLLBACK;
    // delete large object
    int lo_result = lo_unlink(DB::Postgres::get_singleton()->get_handle(), image_oid);
    if(lo_result == -1) {
        neroshop::print(POSTGRESQL_TAG "lo_unlink failed: " + std::string(PQerrorMessage(DB::Postgres::get_singleton()->get_handle())), 1);
        DB::Postgres::get_singleton()->execute("ROLLBACK;"); // abort transaction
        return;
    }   
    // remove image data and other values at a specific index in the arrays
    DB::Postgres::get_singleton()->execute_params("UPDATE images SET name = array_remove(name, name[$1]) WHERE item_id = $2", { std::to_string(index), std::to_string(this->id) });
    DB::Postgres::get_singleton()->execute_params("UPDATE images SET data = array_remove(data, data[$1]) WHERE item_id = $2", { std::to_string(index), std::to_string(this->id) });
    neroshop::print("deleted image at index " + std::to_string(index), 3);
    // remove the image from the database (only if image data and name is empty)
    int image_count = DB::Postgres::get_singleton()->get_integer_params("SELECT cardinality(data) FROM images WHERE item_id = $1", { std::to_string(this->id) });
    if(image_count == 0) {
        DB::Postgres::get_singleton()->execute_params("DELETE FROM images WHERE item_id = $1", { std::to_string(this->id) });
        neroshop::print("item (" + std::to_string(this->id) + ")'s image(s) has been permanently removed from the database", 1);
    }
    // end transaction
    DB::Postgres::get_singleton()->execute("COMMIT;");
#endif    
}
////////////////////
void neroshop::Item::delete_all_upload_images(void) {
    
}
////////////////////
int neroshop::Item::get_image_count() const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    int image_count = DB::Postgres::get_singleton()->get_integer_params("SELECT cardinality(data) FROM images WHERE item_id = $1", { std::to_string(this->id) });
    return image_count;
#endif
    return 0;    
}
////////////////////
////////////////////
////////////////////
void neroshop::Item::set_id(unsigned int id) {
    this->id = id;
}
////////////////////
void neroshop::Item::set_quantity(unsigned int quantity, unsigned int cart_id) {
    // guest cart
    if(cart_id == 0) {
        std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
        DB::Sqlite3 db(cart_file);
        if(!db.table_exists("Cart")) {db.close(); return;}
        db.update("Cart", "item_qty", std::to_string(quantity), "item_id = " + std::to_string(this->id));
        db.close();
        return; // exit function
    }
    // registered_user cart
#if defined(NEROSHOP_USE_POSTGRESQL)    
    DB::Postgres::get_singleton()->execute_params("UPDATE cart_item SET item_qty = $1 WHERE cart_id = $2 AND item_id = $3", { std::to_string(quantity), std::to_string(cart_id), std::to_string(this->id) });
#endif
}
////////////////////
void neroshop::Item::set_quantity(unsigned int item_id, unsigned int quantity, unsigned int cart_id) {
    // guest cart
    if(cart_id == 0) {
        std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
        DB::Sqlite3 db(cart_file);
        if(!db.table_exists("Cart")) {db.close(); return;}
        db.update("Cart", "item_qty", std::to_string(quantity), "item_id = " + std::to_string(item_id));
        db.close();
        return; // exit function
    }
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // registered_user cart
    DB::Postgres::get_singleton()->execute_params("UPDATE cart_item SET item_qty = $1 WHERE cart_id = $2 AND item_id = $3", { std::to_string(quantity), std::to_string(cart_id), std::to_string(item_id) });
#endif
}
////////////////////
void neroshop::Item::set_name(const std::string& name) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "name", DB::Sqlite3::to_sql_string(name), "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
#endif    
}
////////////////////
void neroshop::Item::set_name(unsigned int item_id, const std::string& name) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "name", DB::Sqlite3::to_sql_string(name), "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
#endif    
}
////////////////////
void neroshop::Item::set_description(const std::string& description) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "description", DB::Sqlite3::to_sql_string(description), "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
#endif    
}
////////////////////
void neroshop::Item::set_description(unsigned int item_id, const std::string& description) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "description", DB::Sqlite3::to_sql_string(description), "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
#endif    
}
////////////////////
void neroshop::Item::set_price(double price) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "price", std::to_string(price), "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
#endif    
}
////////////////////
void neroshop::Item::set_price(unsigned int item_id, double price) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "price", std::to_string(price), "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
////////////////////
void neroshop::Item::set_weight(double weight) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "weight", std::to_string(weight), "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
void neroshop::Item::set_weight(unsigned int item_id, double weight) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "weight", std::to_string(weight), "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
void neroshop::Item::set_size(double l, double w, double h) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    std::string size = std::to_string(l) + "x" + std::to_string(w) + "x" + std::to_string(h);
    //std::cout << "item_size set: " << size << std::endl;
    db.update("item", "size", DB::Sqlite3::to_sql_string(size) , "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
void neroshop::Item::set_size(unsigned int item_id, double l, double w, double h) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    std::string size = std::to_string(l) + "x" + std::to_string(w) + "x" + std::to_string(h);
    //std::cout << "item_size set: " << size << std::endl;
    db.update("item", "size", DB::Sqlite3::to_sql_string(size) , "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
////////////////////
void neroshop::Item::set_size(const std::tuple<double, double, double>& size) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    std::string item_size = std::to_string(std::get<0>(size)) + "x" + std::to_string(std::get<1>(size)) + "x" + std::to_string(std::get<2>(size));
    //std::cout << "item_size set: " << item_size << std::endl;
    db.update("item", "size", DB::Sqlite3::to_sql_string(item_size) , "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
void neroshop::Item::set_size(unsigned int item_id, const std::tuple<double, double, double>& size) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    std::string item_size = std::to_string(std::get<0>(size)) + "x" + std::to_string(std::get<1>(size)) + "x" + std::to_string(std::get<2>(size));
    //std::cout << "item_size set: " << item_size << std::endl;
    db.update("item", "size", DB::Sqlite3::to_sql_string(item_size) , "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
void neroshop::Item::set_discount(double discount) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "discount", std::to_string(discount), "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
void neroshop::Item::set_discount(unsigned int item_id, double discount) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "discount", std::to_string(discount), "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
void neroshop::Item::set_discount_by_percentage(double percent) { // converts a percentage to a price then sets the price as the discount
    // convert discount (a percentage) into a decimal
    double num2dec = percent / 100; //std::cout << percent << "% is " << num2dec << std::endl;
    double original_price = get_price();
    std::cout << "original_price: $" << original_price << std::endl;
    double discount = original_price * num2dec; 
    std::cout << "discount: $" << discount/* << " = " << percent << "% of " << original_price*/ << std::endl;
    //double sale_price = original_price - discount;
    //std::cout << "*new* sales_price: $" << sale_price << std::endl;
    ////////////////////////////////
    // sqlite
    ////////////////////////////////
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "discount", std::to_string(discount), "id = " + std::to_string(this->id));
    db.close();    
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////
}
////////////////////
void neroshop::Item::set_discount_by_percentage(unsigned int item_id, double percent) {
    // convert discount (a percentage) into a decimal
    double num2dec = percent / 100; //std::cout << percent << "% is " << num2dec << std::endl;
    double original_price = get_price(item_id);
    std::cout << "original_price: $" << original_price << std::endl;
    double discount = original_price * num2dec; 
    std::cout << "discount: $" << discount/* << " = " << percent << "% of " << original_price*/ << std::endl;
    //double sale_price = original_price - discount;
    //std::cout << "*new* sales_price: $" << sale_price << std::endl;
    ////////////////////////////////
    // sqlite
    ////////////////////////////////
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "discount", std::to_string(discount), "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
void neroshop::Item::set_condition(const std::string& condition) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "condition", DB::Sqlite3::to_sql_string(condition), "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
void neroshop::Item::set_condition(unsigned int item_id, const std::string& condition) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "condition", DB::Sqlite3::to_sql_string(condition), "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
void neroshop::Item::set_product_code(const std::string& product_code) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "product_code", DB::Sqlite3::to_sql_string(product_code), "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
void neroshop::Item::set_product_code(unsigned int item_id, const std::string& product_code) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return;}
    db.update("item", "product_code", DB::Sqlite3::to_sql_string(product_code), "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
}
////////////////////
////////////////////
// getters
////////////////////
////////////////////
unsigned int neroshop::Item::get_id() const {
    return id; // should be 0 by default for unregistered items
}
////////////////////
std::string neroshop::Item::get_name() const {
    /*DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return "";}
    std::string item_name = db.get_column_text("item", "name", "id = " + std::to_string(this->id));
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(this->id) });
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return item_name;
#endif
    return "";    
}
////////////////////
std::string neroshop::Item::get_name(unsigned int item_id) {
    /*DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return "";}
    std::string item_name = db.get_column_text("item", "name", "id = " + std::to_string(item_id));
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    std::string item_name = DB::Postgres::get_singleton()->get_text_params("SELECT name FROM item WHERE id = $1", { std::to_string(item_id) });
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////
    return item_name;     
#endif
    return "";    
}
////////////////////
std::string neroshop::Item::get_description() const {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return "";}
    std::string item_description = db.get_column_text("item", "description", "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return item_description;
}
////////////////////
std::string neroshop::Item::get_description(unsigned int item_id) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return "";}
    std::string item_description = db.get_column_text("item", "description", "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return item_description;
}
////////////////////
double neroshop::Item::get_price() const {
    /*DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return 0.0;}
    double item_price = db.get_column_real("item", "price", "id = " + std::to_string(this->id));
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    double item_price = DB::Postgres::get_singleton()->get_real_params("SELECT price FROM item WHERE id = $1", { std::to_string(this->id) });
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return item_price;
#endif    
    return 0.0;
}
////////////////////
double neroshop::Item::get_price(unsigned int item_id) { // original price (list price)
    /*DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return 0.0;}
    double item_price = db.get_column_real("item", "price", "id = " + std::to_string(item_id));
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    double item_price = DB::Postgres::get_singleton()->get_real_params("SELECT price FROM item WHERE id = $1", { std::to_string(item_id) });
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return item_price;
#endif    
    return 0.0;
}
////////////////////
unsigned int neroshop::Item::get_quantity(unsigned int cart_id) const {
    // guest cart
    if(cart_id == 0) {
        std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
        DB::Sqlite3 db(cart_file);
        if(!db.table_exists("Cart")) {db.close(); return 0;}
        unsigned int quantity = db.get_column_integer("Cart", "item_qty", "item_id = " + std::to_string(this->id));//std::cout << "item_id: " << item_id << " has a quantity of " << item_qty << std::endl;   
        db.close();
        return quantity; // return item_qty then exit function
    }
    // registered_user cart    
#if defined(NEROSHOP_USE_POSTGRESQL)    
    unsigned int quantity = DB::Postgres::get_singleton()->get_integer_params("SELECT item_qty FROM cart_item WHERE cart_id = $1 AND item_id = $2", { std::to_string(cart_id), std::to_string(this->id) });
    return quantity;
#endif
    return 0;    
}
////////////////////
unsigned int neroshop::Item::get_quantity(unsigned int item_id, unsigned int cart_id) {
    // guest cart
    if(cart_id == 0) {
        std::string cart_file = std::string("/home/" + System::get_user() + "/.config/neroshop/") + "cookies.sqlite3";
        DB::Sqlite3 db(cart_file);
        if(!db.table_exists("Cart")) {db.close(); return 0;}
        unsigned int quantity = db.get_column_integer("Cart", "item_qty", "item_id = " + std::to_string(item_id));//std::cout << "item_id: " << item_id << " has a quantity of " << item_qty << std::endl;   
        db.close();
        return quantity; // return item_qty then exit function
    }
    // registered_user cart
#if defined(NEROSHOP_USE_POSTGRESQL)    
    unsigned int quantity = DB::Postgres::get_singleton()->get_integer_params("SELECT item_qty FROM cart_item WHERE cart_id = $1 AND item_id = $2", { std::to_string(cart_id), std::to_string(item_id) });
    return quantity;
#endif    
    return 0;
}
////////////////////
////////////////////
////////////////////
double neroshop::Item::get_weight() const {
    /*DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return 0.0;}
    double item_weight = db.get_column_real("item", "weight", "id = " + std::to_string(this->id));
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    double item_weight = DB::Postgres::get_singleton()->get_real_params("SELECT weight FROM item WHERE id = $1", { std::to_string(this->id) });
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return item_weight;
#endif
    return 0.0;    
}
////////////////////
double neroshop::Item::get_weight(unsigned int item_id) {
    /*DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return 0.0;}
    double item_weight = db.get_column_real("item", "weight", "id = " + std::to_string(item_id));
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    double item_weight = DB::Postgres::get_singleton()->get_real_params("SELECT weight FROM item WHERE id = $1", { std::to_string(item_id) });
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return item_weight;
#endif
    return 0.0;//0.0f;    
}
////////////////////
std::tuple<double, double, double> neroshop::Item::get_size() const {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return std::make_tuple(0, 0, 0);}
    std::vector<std::string> item_size = String::split(db.get_column_text("item", "size", "id = " + std::to_string(this->id)), "x");
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return std::make_tuple(std::stod(item_size[0]), std::stod(item_size[1]), std::stod(item_size[2]));
}
////////////////////
std::tuple<double, double, double> neroshop::Item::get_size(unsigned int item_id) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return std::make_tuple(0, 0, 0);}
    std::vector<std::string> item_size = String::split(db.get_column_text("item", "size", "id = " + std::to_string(item_id)), "x");
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return std::make_tuple(std::stod(item_size[0]), std::stod(item_size[1]), std::stod(item_size[2]));
}
////////////////////
double neroshop::Item::get_discount(unsigned int seller_id) const {
    return get_seller_discount(seller_id); // item discount is determined by seller
}
////////////////////
double neroshop::Item::get_discount(unsigned int item_id, unsigned int seller_id) {
    return get_seller_discount(item_id, seller_id); // item discount is determined by seller
}
////////////////////
std::string neroshop::Item::get_condition() const {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return "";}
    std::string item_condition = db.get_column_text("item", "condition", "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return item_condition;
}
////////////////////
std::string neroshop::Item::get_condition(unsigned int item_id) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return "";}
    std::string item_condition = db.get_column_text("item", "condition", "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return item_condition;
}
////////////////////
//std::string get_subcategory() const {/*return subcategory;*/}
////////////////////
std::string neroshop::Item::get_product_code() const {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return "";}
    std::string item_product_code = db.get_column_text("item", "product_code", "id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return item_product_code;
}
////////////////////
std::string neroshop::Item::get_product_code(unsigned int item_id) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("item")) {db.close(); return "";}
    std::string item_product_code = db.get_column_text("item", "product_code", "id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return item_product_code;
}
////////////////////
////////////////////
// ratings
////////////////////
int neroshop::Item::get_ratings_count() const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");	
    if(!DB::Postgres::get_singleton()->table_exists("item_ratings")) {/*//DB::Postgres::get_singleton()->finish();*/ return 0;}
    unsigned int ratings_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM item_ratings WHERE item_id = $1", { std::to_string(this->id) });    
	//DB::Postgres::get_singleton()->finish();
    return ratings_count;
#endif
    return 0;    
}
////////////////////
int neroshop::Item::get_ratings_count(unsigned int item_id) {
#if defined(NEROSHOP_USE_POSTGRESQL)
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");	
    if(!DB::Postgres::get_singleton()->table_exists("item_ratings")) {/*//DB::Postgres::get_singleton()->finish();*/ return 0;}
    unsigned int ratings_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM item_ratings WHERE item_id = $1", { std::to_string(item_id) });    
	//DB::Postgres::get_singleton()->finish();
    return ratings_count;
#endif
    return 0;  
}
////////////////////
int neroshop::Item::get_star_count(int star_number) {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // star ratings must be between 1 and 5
    if(star_number > 5) star_number = 5;
    if(star_number < 1) star_number = 1;
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    if(!DB::Postgres::get_singleton()->table_exists("item_ratings")) { return 0; }
    unsigned int star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(this->id), std::to_string(star_number) });
    //DB::Postgres::get_singleton()->finish();
    return star_count;
#endif
    return 0;    
}
////////////////////
int neroshop::Item::get_star_count(unsigned int item_id, int star_number) {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // star ratings must be between 1 and 5
    if(star_number > 5) star_number = 5;
    if(star_number < 1) star_number = 1;
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    if(!DB::Postgres::get_singleton()->table_exists("item_ratings")) { return 0; }
    unsigned int star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(item_id), std::to_string(star_number) });
    //DB::Postgres::get_singleton()->finish();
    return star_count;
#endif
    return 0;    
}
////////////////////
float neroshop::Item::get_average_stars() const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    if(!DB::Postgres::get_singleton()->table_exists("item_ratings")) { return 0.0f; }
    // get number of star ratings (for a specific item_id)
    unsigned int total_star_ratings = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM item_ratings WHERE item_id = $1", { std::to_string(this->id) });
    if(total_star_ratings == 0) { /*std::cout << "This item has no star ratings\n";*/ return 0.0f; }
    // get number of 1, 2, 3, 4, and 5 star_ratings
    int one_star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(this->id), std::to_string(1) });
    int two_star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(this->id), std::to_string(2) });
    int three_star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(this->id), std::to_string(3) });
    int four_star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(this->id), std::to_string(4) });
    int five_star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(this->id), std::to_string(5) });
    // now calculate average stars
    // 3 total star ratings:(5, 3, 4) // average = (5 + 3 + 4) / 3 = 4 stars        
    float average_stars = (
        (1 * static_cast<float>(one_star_count)) + 
        (2 * static_cast<float>(two_star_count)) + 
        (3 * static_cast<float>(three_star_count)) + 
        (4 * static_cast<float>(four_star_count)) + 
        (5 * static_cast<float>(five_star_count))) / total_star_ratings;
    ////////////////////
    //DB::Postgres::get_singleton()->finish();
    return average_stars;
#endif    
    return 0.0f;
}
////////////////////
float neroshop::Item::get_average_stars(unsigned int item_id) {
#if defined(NEROSHOP_USE_POSTGRESQL)
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");
    if(!DB::Postgres::get_singleton()->table_exists("item_ratings")) { return 0.0f; }
    // get number of star ratings (for a specific item_id)
    unsigned int total_star_ratings = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(*) FROM item_ratings WHERE item_id = $1", { std::to_string(item_id) });
    if(total_star_ratings == 0) { /*std::cout << "This item has no star ratings\n";*/ return 0.0f; }
    // get number of 1, 2, 3, 4, and 5 star_ratings
    int one_star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(item_id), std::to_string(1) });
    int two_star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(item_id), std::to_string(2) });
    int three_star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(item_id), std::to_string(3) });
    int four_star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(item_id), std::to_string(4) });
    int five_star_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(stars) FROM item_ratings WHERE item_id = $1 AND stars = $2", { std::to_string(item_id), std::to_string(5) });
    // now calculate average stars
    // 3 total star ratings:(5, 3, 4) // average = (5 + 3 + 4) / 3 = 4 stars        
    float average_stars = (
        (1 * static_cast<float>(one_star_count)) + 
        (2 * static_cast<float>(two_star_count)) + 
        (3 * static_cast<float>(three_star_count)) + 
        (4 * static_cast<float>(four_star_count)) + 
        (5 * static_cast<float>(five_star_count))) / total_star_ratings;
    ////////////////////
    //DB::Postgres::get_singleton()->finish();
    return average_stars;
#endif    
    return 0.0f;
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
// seller
////////////////////
////////////////////
double neroshop::Item::get_seller_price(unsigned int seller_id) const {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("inventory")) {db.close(); return 0.00;} // make sure table exists first
    double seller_price = 0.00;
    if(seller_id > 0) seller_price = db.get_column_real("inventory", "seller_price", "item_id = " + std::to_string(this->id) + " AND seller_id = " + std::to_string(seller_id)); // get a specific seller's price for this item
    if(seller_id <= 0) seller_price = db.get_column_real("inventory", "seller_price", "item_id = " + std::to_string(this->id)); // get a random seller's price for this item
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return seller_price;
}
////////////////////
double neroshop::Item::get_seller_price(unsigned int item_id, unsigned int seller_id) {// seller price (sales price)
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("inventory")) {db.close(); return 0.00;} // make sure table exists first
    double seller_price = 0.00;
    if(seller_id > 0) seller_price = db.get_column_real("inventory", "seller_price", "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(seller_id));
    if(seller_id <= 0) seller_price = db.get_column_real("inventory", "seller_price", "item_id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return seller_price;
}
////////////////////
double neroshop::Item::get_seller_discount(unsigned int seller_id) const {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("inventory")) {db.close(); return 0.00;} // make sure table exists first
    double seller_discount = 0.00;
    if(seller_id > 0) seller_discount = db.get_column_real("inventory", "seller_discount", "item_id = " + std::to_string(this->id) + " AND seller_id = " + std::to_string(seller_id));
    if(seller_id <= 0) seller_discount = db.get_column_real("inventory", "seller_discount", "item_id = " + std::to_string(this->id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return seller_discount;
}
////////////////////
double neroshop::Item::get_seller_discount(unsigned int item_id, unsigned int seller_id) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("inventory")) {db.close(); return 0.00;} // make sure table exists first
    double seller_discount = 0.00;
    if(seller_id > 0) seller_discount = db.get_column_real("inventory", "seller_discount", "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(seller_id));
    if(seller_id <= 0) seller_discount = db.get_column_real("inventory", "seller_discount", "item_id = " + std::to_string(item_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return seller_discount;
}
////////////////////
std::string neroshop::Item::get_seller_condition(unsigned int seller_id) const {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("inventory")) {db.close(); return "";} // make sure table exists first
    std::string seller_condition = db.get_column_text("inventory", "seller_condition", "item_id = " + std::to_string(this->id) + " AND seller_id = " + std::to_string(seller_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return seller_condition;
}
////////////////////
std::string neroshop::Item::get_seller_condition(unsigned int item_id, unsigned int seller_id) {
    DB::Sqlite3 db("neroshop.db");
    if(!db.table_exists("inventory")) {db.close(); return "";} // make sure table exists first
    std::string seller_condition = db.get_column_text("inventory", "seller_condition", "item_id = " + std::to_string(item_id) + " AND seller_id = " + std::to_string(seller_id));
    db.close();
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return seller_condition;
#endif
    return "";
}
////////////////////
unsigned int neroshop::Item::get_stock_quantity() const {
    /*DB::Sqlite3 db;if(!db.open("neroshop.db")) {std::cout << "Could not open sql database" << std::endl; return 0;}
    if(!db.table_exists("inventory")) {db.close(); return 0;} // make sure table exists first
    int stock_qty = db.get_column_integer("inventory", "stock_qty", "item_id=" + std::to_string(this->id)); // check stock_quantity
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    int stock_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND stock_qty > 0", { std::to_string(this->id) });
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return stock_qty;
#endif
    return 0;    
}
//////////////////// 
unsigned int neroshop::Item::get_stock_quantity(unsigned int item_id) {
    /*DB::Sqlite3 db;if(!db.open("neroshop.db")) {std::cout << "Could not open sql database" << std::endl; return 0;}
    if(!db.table_exists("inventory")) {db.close(); return 0;} // make sure table exists first
    int stock_qty = db.get_column_integer("inventory", "stock_qty", "item_id=" + std::to_string(item_id)); // check stock_quantity
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    int stock_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND stock_qty > 0", { std::to_string(item_id) });
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////
    return stock_qty;
#endif
    return 0;    
}
////////////////////
unsigned int neroshop::Item::get_seller_count() const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    int seller_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(seller_id) FROM inventory WHERE item_id = $1;", { std::to_string(this->id) });
    return seller_count;
#endif
    return 0;    
}
////////////////////
unsigned int neroshop::Item::get_seller_count(unsigned int item_id) {
#if defined(NEROSHOP_USE_POSTGRESQL)
    int seller_count = DB::Postgres::get_singleton()->get_integer_params("SELECT COUNT(seller_id) FROM inventory WHERE item_id = $1;", { std::to_string(item_id) });
    return seller_count;
#endif
    return 0;    
}
////////////////////
unsigned int neroshop::Item::get_seller_id() const { // might need a little more work
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // prioritize the seller with the most positive (or good) ratings that sells this item
    int seller_id = DB::Postgres::get_singleton()->get_integer_params("SELECT inventory.seller_id FROM inventory JOIN seller_ratings ON inventory.seller_id = seller_ratings.seller_id WHERE inventory.item_id = $1 AND score = 1 GROUP BY inventory.seller_id ORDER BY COUNT(score) DESC LIMIT 1;", { std::to_string(this->id) });//int seller_id = DB::Postgres::get_singleton()->get_integer_params("SELECT seller_id FROM inventory WHERE item_id = $1 AND stock_qty > 0;", { std::to_string(this->id) });
    // if the seller_ratings table does not exist yet or the seller has not yet been rated then we can just get any random seller (or maybe I should get the first seller to list this item??)
    if(seller_id == 0) seller_id = DB::Postgres::get_singleton()->get_integer_params("SELECT seller_id FROM inventory WHERE item_id = $1;", { std::to_string(this->id) });//int seller_id = DB::Postgres::get_singleton()->get_integer_params("SELECT seller_id FROM inventory WHERE item_id = $1 AND stock_qty > 0;", { std::to_string(this->id) });
    return seller_id;
#endif
    return 0;    
}
////////////////////
unsigned int neroshop::Item::get_seller_id(unsigned int item_id) { // might need a little more work
#if defined(NEROSHOP_USE_POSTGRESQL)    
    // prioritize the seller with the most positive (or good) ratings that sells this item
    int seller_id = DB::Postgres::get_singleton()->get_integer_params("SELECT inventory.seller_id FROM inventory JOIN seller_ratings ON inventory.seller_id = seller_ratings.seller_id WHERE inventory.item_id = $1 AND score = 1 GROUP BY inventory.seller_id ORDER BY COUNT(score) DESC LIMIT 1;", { std::to_string(item_id) });//int seller_id = DB::Postgres::get_singleton()->get_integer_params("SELECT seller_id FROM inventory WHERE item_id = $1 AND stock_qty > 0;", { std::to_string(this->id) });
    // if the seller_ratings table does not exist yet or the seller has not yet been rated then we can just get any random seller (or maybe I should get the first seller to list this item??)
    if(seller_id == 0) seller_id = DB::Postgres::get_singleton()->get_integer_params("SELECT seller_id FROM inventory WHERE item_id = $1;", { std::to_string(item_id) });//int seller_id = DB::Postgres::get_singleton()->get_integer_params("SELECT seller_id FROM inventory WHERE item_id = $1 AND stock_qty > 0;", { std::to_string(item_id) });
    return seller_id;
#endif
    return 0;    
}
////////////////////
std::vector<unsigned int> neroshop::Item::get_seller_ids() const {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // DISTINCT is used here to make sure that each seller_id is unique (is stored only once)
    std::string command = "SELECT DISTINCT seller_id FROM inventory WHERE item_id = $1";// AND stock_qty > 0;";
    std::vector<const char *> param_values = { std::to_string(this->id).c_str() };
    PGresult * result = PQexecParams(DB::Postgres::get_singleton()->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("Item::get_seller_ids(): No sellers found", 2);        
        PQclear(result);
        return {}; // exit so that we don't double free "result"
    }
    int rows = PQntuples(result);
    std::vector<unsigned int> seller_ids = {};
    for(int i = 0; i < rows; i++) {
        int seller_id = std::stoi(PQgetvalue(result, i, 0));
        seller_ids.push_back(seller_id);//std::cout << "sellers that have listed this item: " << seller_id << std::endl;
    }
    PQclear(result); // free result
    return seller_ids;
#endif
    return {};    
}
////////////////////
std::vector<unsigned int> neroshop::Item::get_seller_ids(unsigned int item_id) {
#if defined(NEROSHOP_USE_POSTGRESQL)
    // DISTINCT is used here to make sure that each seller_id is unique (is stored only once)
    std::string command = "SELECT DISTINCT seller_id FROM inventory WHERE item_id = $1";// AND stock_qty > 0;";
    std::vector<const char *> param_values = { std::to_string(item_id).c_str() };
    PGresult * result = PQexecParams(DB::Postgres::get_singleton()->get_handle(), command.c_str(), 1, nullptr, param_values.data(), nullptr, nullptr, 0);
    if (PQresultStatus(result) != PGRES_TUPLES_OK) {
        neroshop::print("Item::get_seller_ids(): No sellers found", 2);        
        PQclear(result);
        return {}; // exit so that we don't double free "result"
    }
    int rows = PQntuples(result);
    std::vector<unsigned int> seller_ids = {};
    for(int i = 0; i < rows; i++) {
        int seller_id = std::stoi(PQgetvalue(result, i, 0));
        seller_ids.push_back(seller_id);//std::cout << "sellers that have listed this item: " << seller_id << std::endl;
    }
    PQclear(result); // free result
    return seller_ids;
#endif
    return {};    
}
////////////////////
////////////////////
////////////////////
bool neroshop::Item::is_registered() const {
    /*DB::Sqlite3 db;if(!db.open("neroshop.db")) {std::cout << "Could not open sql database" << std::endl; return false;}
    if(!db.table_exists("item")) {db.close(); return false;} // make sure table exists first
    int id = db.get_column_integer("item", "id", "id=" + std::to_string(this->id));
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    int id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM item WHERE id = $1", { std::to_string(this->id) });
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////
    return (id > 0);
#endif
    return false;    
}
////////////////////
bool neroshop::Item::is_registered(unsigned int item_id) {
    /*DB::Sqlite3 db;if(!db.open("neroshop.db")) {std::cout << "Could not open sql database" << std::endl; return false;}
    if(!db.table_exists("item")) {db.close(); return false;} // make sure table exists first
    int id = db.get_column_integer("item", "id", "id=" + std::to_string(item_id));
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    int id = DB::Postgres::get_singleton()->get_integer_params("SELECT id FROM item WHERE id = $1", { std::to_string(item_id) });
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return (id > 0);
#endif
    return false;    
}
////////////////////
////////////////////
bool neroshop::Item::in_stock() const {
    /*DB::Sqlite3 db;if(!db.open("neroshop.db")) {std::cout << "Could not open sql database" << std::endl; return false;}
    if(!db.table_exists("inventory")) {db.close(); return false;} // make sure table exists first
    int stock_qty = db.get_column_integer("inventory", "stock_qty", "item_id=" + std::to_string(this->id)); // check stock_quantity
    if(stock_qty < 1) {db.close();return false;}
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    int stock_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND stock_qty > 0", { std::to_string(this->id) }); // check stock_qty regardless of who is selling said item
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////    
    return (stock_qty != 0);
#endif
    return false;    
}
////////////////////
bool neroshop::Item::in_stock(unsigned int item_id) {
    /*DB::Sqlite3 db;if(!db.open("neroshop.db")) {std::cout << "Could not open sql database" << std::endl; return false;}
    if(!db.table_exists("inventory")) {db.close(); return false;} // make sure table exists first
    int stock_qty = db.get_column_integer("inventory", "stock_qty", "item_id=" + std::to_string(item_id)); // check stock_quantity
    if(stock_qty < 1) {db.close();return false;}
    db.close();*/
    ////////////////////////////////
    // postgresql
    ////////////////////////////////
#if defined(NEROSHOP_USE_POSTGRESQL)    
    //DB::Postgres::get_singleton()->connect("host=127.0.0.1 port=5432 user=postgres password=postgres dbname=neroshoptest");    
    int stock_qty = DB::Postgres::get_singleton()->get_integer_params("SELECT stock_qty FROM inventory WHERE item_id = $1 AND stock_qty > 0", { std::to_string(item_id) }); // check stock_qty regardless of who is selling said item
    // if item is out of stock, look for other sellers that have the same exact item (but it MUST be in stock)
    //DB::Postgres::get_singleton()->finish();
    ////////////////////////////////
    return (stock_qty != 0);
#endif
    return false;    
}
////////////////////
////////////////////
bool neroshop::Item::in_cart(/*unsigned int cart_id*/) const {
    std::string user = System::get_user();
    std::string cart_file = std::string("/home/" + user + "/.config/neroshop/") + "cookies.sqlite3";
    DB::Sqlite3 db; if(!db.open(cart_file)) {std::cout << "Could not open sql database" << std::endl; return false;}
    if(!db.table_exists("Cart")) {db.close(); return false;} // make sure table exists first
    int id = db.get_column_integer("Cart", "item_id", "item_id = " + std::to_string(this->id));
    //    "cart_id = " + std::to_string(cart_id) + " AND item_id = " + std::to_string(this->id));
    if(id != this->id) {db.close(); return false;} // if this->id matches the item_id in table Cart, return true
    db.close();
    return true;
}
////////////////////
bool neroshop::Item::in_cart(unsigned int item_id/*, unsigned int cart_id*/) {
    std::string user = System::get_user();
    std::string cart_file = std::string("/home/" + user + "/.config/neroshop/") + "cookies.sqlite3";
    DB::Sqlite3 db; if(!db.open(cart_file)) {std::cout << "Could not open sql database" << std::endl; return false;}
    if(!db.table_exists("Cart")) {db.close(); return false;} // make sure table exists first
    int id = db.get_column_integer("Cart", "item_id", "item_id = " + std::to_string(item_id));
    //    "cart_id = " + std::to_string(cart_id) + " AND item_id = " + std::to_string(item_id));
    if(id != item_id) {db.close(); return false;} // if item_id matches the item_id in table Cart, return true
    db.close();
    return true;
}
////////////////////
////////////////////
////////////////////
////////////////////
void neroshop::Item::show_info() {
    std::cout << "Item name: " << get_name() << std::endl;
    std::cout << "id: " << get_id() << std::endl;
    std::cout << "desc: " << get_description() << std::endl;
    //std::cout << "quantity: " << get_quantity() << std::endl;
    std::cout << "price: " << get_price() << std::endl;
    std::cout << "weight: " << get_weight() << std::endl;
    //std::cout << "size: " << item.get_size() << std::endl;
    //std::cout << "discount: " << get_discount() << std::endl;
    std::cout << "condition: " << get_condition() << std::endl;
    std::cout << "product_code: " << get_product_code() << std::endl;
    /*std::cout << ": " << item.get_() << std::endl;
    std::cout << ": " << item.get_() << std::endl;
    std::cout << ": " << item.get_() << std::endl;
    std::cout << ": " << item.get_() << std::endl;
    std::cout << ": " << item.get_() << std::endl;
    std::cout << ": " << item.get_() << std::endl;
    std::cout << ": " << item.get_() << std::endl;
    std::cout << ": " << item.get_() << std::endl;
    std::cout << ": " << item.get_() << std::endl;*/
}
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
////////////////////
