// Translated from solution.cpp.

var m: dynamic = cpp_array(303300, 2);

var p1: dynamic = cpp_uninitialized();

var p2: dynamic = cpp_uninitialized();

var offset: dynamic = 151500;

var off: dynamic = 120000;

var resa: dynamic = cpp_array(303300);

var mod: dynamic = (1e9 + 7);

func solve(a: dynamic, k: dynamic, t: dynamic) -> dynamic
{
  p1 = (m[0] + offset);
  p2 = (m[1] + offset);
  memset(m, 0, cpp_sizeof((m)));
  p1[a] = 1;
  {
    var i: dynamic = 0;
    while ((i < t))
    {
      if ((i == 2))
      {
        var f: dynamic = 0;
      }
      var tmp: dynamic = 0;
      memset((p2 - offset), 0, cpp_sizeof((m[0])));
      {
        var j: dynamic = (-off);
        while ((j < off))
        {
          p2[j] = tmp;
          tmp = (((((((((tmp - p1[(j - k)])) % mod) + mod)) % mod) + p1[((j + k) + 1)])) % mod);
          j += 1;
        }
      }
      swap(p1, p2);
      i += 1;
    }
  }
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(a, b, k, t);
  solve(a, k, t);
  memcpy(resa, (p1 - offset), cpp_sizeof((m[0])));
  solve(b, k, t);
  var res: dynamic = 0;
  var tmp: dynamic = 0;
  {
    var i: dynamic = (-off);
    while ((i < off))
    {
      tmp = (((tmp + p1[i])) % mod);
      res = (((res + (((cpp_cast(resa[((i + offset) + 1)]) * tmp)) % mod))) % mod);
      i += 1;
    }
  }
  printf("%d\n", res);
  return 0;
}
