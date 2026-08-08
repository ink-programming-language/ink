// Translated from solution.cpp.

func main()
{
  var bs = cpp_construct(0);
  var q: dynamic;
  read(q);
  {
    var i = 0;
    while ((i < q))
    {
      var op: dynamic;
      var k: dynamic;
      read(op);
      var __cpp_switch_1 = op;
      if (__cpp_switch_1 == 0)
      {
        read(k);
        write(bs.test(k), "\n");
        break;
      }
      else if (__cpp_switch_1 == 1)
      {
        read(k);
        bs.set(k);
        break;
      }
      else if (__cpp_switch_1 == 2)
      {
        read(k);
        bs.reset(k);
        break;
      }
      else if (__cpp_switch_1 == 3)
      {
        read(k);
        bs.flip(k);
        break;
      }
      else if (__cpp_switch_1 == 4)
      {
        write(bs.all(), "\n");
        break;
      }
      else if (__cpp_switch_1 == 5)
      {
        write(bs.any(), "\n");
        break;
      }
      else if (__cpp_switch_1 == 6)
      {
        write(bs.none(), "\n");
        break;
      }
      else if (__cpp_switch_1 == 7)
      {
        write(bs.count(), "\n");
        break;
      }
      else if (__cpp_switch_1 == 8)
      {
        write(bs.to_ullong(), "\n");
        break;
      }
      i += 1;
    }
  }
  return 0;
}
