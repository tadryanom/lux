
DefinitionBlock("acpi.aml", "DSDT", 1, "TEST!", "", 1)
{
	Scope(_SB)
	{
		Device(HPET)
		{
			Name(_HID, EisaID("PNP0103"))
			Name(_UID, Zero)

			Name(_CRS, ResourceTemplate()
			{
				IRQNoFlags() { 4, 9 }
				Memory32Fixed (ReadOnly, 0xFE48, 65)
			})
		}
	}
}


