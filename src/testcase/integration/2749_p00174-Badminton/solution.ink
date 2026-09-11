// Translated from solution.cpp.

func main(argument_0: dynamic) -> dynamic
{
  ios_base.sync_with_stdio();
  var inp: dynamic = cpp_uninitialized();
  var As: dynamic = cpp_uninitialized();
  var Bs: dynamic = cpp_uninitialized();
  while (1)
  {
    read(inp);
    if ((inp == "0"))
    {
      break;
    }
    As = 0;
    Bs = 0;
    {
      var i: dynamic = 1;
      while ((i < inp.length()))
      {
        if ((inp[i] == cpp_char("A")))
        {
          As += 1;
        } else
        {
          Bs += 1;
        }
        i += 1;
      }
    }
    if ((max(As, Bs) == As))
    {
      As += 1;
    } else
    {
      Bs += 1;
    }
    write(As, " ", Bs, "\n");
  }
  return 0;
}
