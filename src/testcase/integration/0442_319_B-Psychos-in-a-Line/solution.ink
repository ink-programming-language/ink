// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_array(100005);

var x: dynamic = cpp_uninitialized();

var k: dynamic = cpp_array(100005);

var res: dynamic = cpp_uninitialized();

var last: dynamic = cpp_uninitialized();

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x);
      if ((s.empty() || (x > s.top())))
      {
        s.push(x);
      } else
      {
        if ((last > x))
        {
          m[x] = 1;
          k[x] = last;
        } else
        {
          var lastVic: dynamic = last;
          while (((k[lastVic] < x) || (m[k[lastVic]] == m[lastVic])))
          {
            lastVic = k[lastVic];
          }
          m[x] = (m[lastVic] + 1);
          k[x] = k[lastVic];
        }
      }
      res = max(res, m[x]);
      last = x;
      i += 1;
    }
  }
  write(res, "\n");
  return 0;
}
