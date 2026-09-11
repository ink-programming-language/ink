// Translated from solution.cpp.

func main() -> dynamic
{
  var bs: dynamic = cpp_construct(0);
  var q: dynamic = cpp_uninitialized();
  read(q);
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      var op: dynamic = cpp_uninitialized();
      var k: dynamic = cpp_uninitialized();
      read(op);
      var __cpp_switch_1: dynamic = op;
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
