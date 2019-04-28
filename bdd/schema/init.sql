DROP TABLE building CASCADE;
DROP TABLE architect CASCADE;
DROP TABLE image CASCADE;
DROP TABLE architectural_element CASCADE;
DROP TABLE model CASCADE;
DROP TABLE address CASCADE;
DROP TABLE image_of_building CASCADE;
DROP TABLE element_of_building CASCADE;
DROP TABLE image_of_element CASCADE;
DROP TABLE model_of_element CASCADE;



CREATE TABLE building (
       b_id            serial,
       b_year          integer,
       b_style         text,
       PRIMARY KEY (b_id),
       CONSTRAINT valid_year CHECK (b_year > 0)
);

CREATE TABLE architect (
       a_name          text,
       b_id            serial REFERENCES building (b_id) ON DELETE CASCADE,
       PRIMARY KEY(a_name, b_id)
);

CREATE TABLE image (
       i_id               serial,
       i_path             text UNIQUE NOT NULL,
       i_tpath            text UNIQUE,
       i_light_cond       text, --This probably should have a constraint.
       i_shooting_date    date,
       i_coordinates      point,
       i_focal_distance   real,
       i_pose             real, --This type should change
       i_instrinsic       real, --This type should change
       PRIMARY KEY (i_id)
);

CREATE TABLE architectural_element (
       e_id            serial,
       e_name          text NOT NULL,
       e_style         text NOT NULL,
       PRIMARY KEY (e_id)
);

CREATE TABLE model (
       m_id       serial,
       m_method   text,
       m_texture  text,
       m_sw       text NOT NULL,
       m_type     text NOT NULL,
       PRIMARY KEY (m_id)
);

CREATE TABLE address (
       ad_id                  serial,
       b_id                   serial REFERENCES building (b_id) ON DELETE CASCADE,
       ad_street_num          integer NOT NULL,
       ad_street              text    NOT NULL,
       ad_city                text    NOT NULL,
       ad_postal_code         text    NOT NULL,
       ad_country             text    NOT NULL,
       PRIMARY KEY (ad_id)
);

CREATE TABLE image_of_building (
       b_id  serial REFERENCES building (b_id) ON DELETE CASCADE,
       i_id  serial REFERENCES image (i_id) ON DELETE CASCADE,
       iob_roi      box NOT NULL,
       PRIMARY KEY (b_id, i_id)
);

CREATE TABLE element_of_building (
       b_id  serial REFERENCES building (b_id) ON DELETE CASCADE,
       e_id  serial REFERENCES architectural_element (e_id) ON DELETE CASCADE,
       PRIMARY KEY (b_id, e_id)
);

CREATE TABLE image_of_element (
       e_id  serial REFERENCES architectural_element (e_id) ON DELETE CASCADE,
       i_id  serial REFERENCES image (i_id) ON DELETE CASCADE,
       ioe_roi      box NOT NULL,
       PRIMARY KEY (e_id, i_id)
);

CREATE TABLE model_of_element (
       e_id  serial REFERENCES architectural_element (e_id) ON DELETE CASCADE,
       m_id  serial REFERENCES model (m_id) ON DELETE CASCADE,
       PRIMARY KEY (e_id, m_id)
)
