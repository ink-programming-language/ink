// Translated from solution.cpp.

func main()
{
  var s = cpp_array(5);
  read(s);
  var o = 0;
  {
    var i = 0;
    while ((i < 4))
    {
      if ((s[i] == cpp_char("-")))
      {
        cpp_update(o, "--") -= 1;
      }
      o += 1;
      i += 1;
    }
  }
  write(o);
}
