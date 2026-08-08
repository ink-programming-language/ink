// Translated from solution.cpp.

var n: dynamic;

var m = cpp_array(100005);

var x: dynamic;

var k = cpp_array(100005);

var res: dynamic;

var last: dynamic;

var s: dynamic;

func main()
{
  read(n);
  {
    var i = 0;
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
          var lastVic = last;
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
