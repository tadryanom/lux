
DefinitionBlock("acpi.aml", "DSDT", 1, "TEST!", "", 1)
{
	Scope(_SB)
	{
		OperationRegion(COM, SystemIO, 0x03F8, One)
		Field (COM, ByteAcc, NoLock, Preserve)
		{
			COM1, 8
		}
	}
}


