// Translated from solution.cpp.

var int_cpp: dynamic = dynamic;

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var a: dynamic = cpp_uninitialized();
    var b: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    read(a, b, k);
    var s: dynamic = cpp_array(k);
    var boy: dynamic = cpp_array(a);
    var girl: dynamic = cpp_array(b);
    {
      var i: dynamic = 0;
      while ((i < a))
      {
        boy[i] = 0;
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < b))
      {
        girl[i] = 0;
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < k))
      {
        read(s[i].first);
        boy[(s[i].first - 1)] += 1;
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < k))
      {
        read(s[i].second);
        girl[(s[i].second - 1)] += 1;
        i += 1;
      }
    }
    var bs: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < a))
      {
        bs = (bs + ((((boy[i] * ((boy[i] - 1)))) / 2)));
        i += 1;
      }
    }
    var gs: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < b))
      {
        gs = (gs + ((((girl[i] * ((girl[i] - 1)))) / 2)));
        i += 1;
      }
    }
    var k1: dynamic = ((((k) * ((k - 1)))) / 2);
    write((((k1 - gs) - bs)), "\n");
  }
}
