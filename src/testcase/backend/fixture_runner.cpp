extern "C" int ink_fixture_entry();

int main()
{
  return ink_fixture_entry() == 42 ? 0 : 1;
}
