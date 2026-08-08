// Translated from solution.cpp.

var m = cpp_array(303300, 2);

var p1: dynamic;

var p2: dynamic;

var offset = 151500;

var off = 120000;

var resa = cpp_array(303300);

var mod = (1e9 + 7);

func solve(a: dynamic, k: dynamic, t: dynamic)
{
  p1 = (m[0] + offset);
  p2 = (m[1] + offset);
  memset(m, 0, cpp_sizeof((m)));
  p1[a] = 1;
  {
    var i = 0;
    while ((i < t))
    {
      if ((i == 2))
      {
        var f = 0;
      }
      var tmp = 0;
      memset((p2 - offset), 0, cpp_sizeof((m[0])));
      {
        var j = (-off);
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

func main()
{
  var a: dynamic;
  var b: dynamic;
  var k: dynamic;
  var t: dynamic;
  read(a, b, k, t);
  solve(a, k, t);
  memcpy(resa, (p1 - offset), cpp_sizeof((m[0])));
  solve(b, k, t);
  var res = 0;
  var tmp = 0;
  {
    var i = (-off);
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
