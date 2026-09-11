// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(l, r);
  a = (r - l);
  if (((r - l) < 100))
  {
    var mp: dynamic = cpp_uninitialized();
    var it: dynamic = cpp_uninitialized();
    {
      var i: dynamic = l;
      while ((i <= r))
      {
        a = i;
        {
          var x: dynamic = 2;
          while (((x * x) <= a))
          {
            if (((a % x) == 0))
            {
              mp[(a / x)] += 1;
              mp[x] += 1;
            }
            x += 1;
          }
        }
        if ((a > 1))
        {
          mp[a] += 1;
        }
        i += 1;
      }
    }
    {
      it = mp.begin();
      while ((it != mp.end()))
      {
        b = max(b, it->second);
        it += 1;
      }
    }
    {
      it = mp.begin();
      while ((it != mp.end()))
      {
        if ((it->second == b))
        {
          write(it->first);
          return 0;
        }
        it += 1;
      }
    }
  } else
  {
    write(2);
  }
}
