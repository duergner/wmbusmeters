/*
 Copyright (C) 2020-2022 Fredrik Öhrström (gpl-3.0-or-later)

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include"meters_common_implementation.h"

namespace
{
    struct Driver : public virtual MeterCommonImplementation
    {
        Driver(MeterInfo &mi, DriverInfo &di);
    };

    static bool ok = registerDriver([](DriverInfo&di)
    {
        di.setName("lansenwl");
        di.setDefaultFields("name,id,status,timestamp");
        di.setMeterType(MeterType::WaterLeakageDetector);
        di.addLinkMode(LinkMode::T1);
        di.setConstructor([](MeterInfo& mi, DriverInfo& di){ return shared_ptr<Meter>(new Driver(mi, di)); });
        di.addDetection(MANUFACTURER_LAS,  0x1e,  0x07);
    });

    Driver::Driver(MeterInfo &mi, DriverInfo &di) : MeterCommonImplementation(mi, di)
    {
        addStringFieldWithExtractorAndLookup(
            "status",
            "The state (OK/WATER_PORT1/WATER_PORT2) for the leakage detection.",
            DEFAULT_PRINT_PROPERTIES,
            FieldMatcher::build()
            .set(MeasurementType::Instantaneous)
            .set(VIFRange::DigitalInput),
            Translate::Lookup()
            .add(Translate::Rule("INPUT_BITS", Translate::Type::IndexToString)
                 .set(MaskBits(0xffff))
                 .add(Translate::Map(0x00 ,"NO_LEAKAGE", TestBit::Set))
                 .add(Translate::Map(0x01 ,"LEAKAGE_PORT1", TestBit::Set))
                 .add(Translate::Map(0x02 ,"LEAKAGE_PORT2", TestBit::Set))
                ));

        addStringFieldWithExtractorAndLookup(
            "error_flags",
            "Error flags.",
            PrintProperty::STATUS | PrintProperty::INCLUDE_TPL_STATUS,
            FieldMatcher::build()
            .set(MeasurementType::Instantaneous)
            .set(VIFRange::ErrorFlags),
            Translate::Lookup()
            .add(Translate::Rule("ERROR_FLAGS", Translate::Type::BitToString)
                 .set(MaskBits(0xffff))
                 .add(Translate::Map(0x01 ,"SABOTAGE", TestBit::Set))
                 .add(Translate::Map(0x02 ,"BATTERY_LOW", TestBit::Set))
                 .set(DefaultMessage("OK"))
                ));

    }
}
