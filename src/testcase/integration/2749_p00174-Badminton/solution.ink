// Translated from solution.cpp.

func main(argument_0: dynamic)
{
  ios_base.sync_with_stdio();
  var inp: dynamic;
  var As: dynamic;
  var Bs: dynamic;
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
      var i = 1;
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
