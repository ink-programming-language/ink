// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(500);

var b: dynamic = cpp_array(500);

var ans: dynamic = cpp_array(4, 500);

var l: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var smm: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var flag: dynamic = 0;

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  read(n);
  memset(b, 0, cpp_sizeof(b));
  memset(a, 0, cpp_sizeof(a));
  {
    var i: dynamic = (1);
    while ((i <= (n)))
    {
      read(a[i]);
      if ((a[i] != 0))
      {
        flag = 1;
      }
      b[i] = (b[(i - 1)] + a[i]);
      i += 1;
    }
  }
  if ((flag == 0))
  {
    write("NO", "\n");
  } else
  {
    write("YES", "\n");
    k = 0;
    r = n;
    l = 1;
    smm = 0;
    while ((l <= n))
    {
      r = n;
      smm = (b[r] - b[(l - 1)]);
      while ((smm == 0))
      {
        r -= 1;
        smm = (b[r] - b[(l - 1)]);
      }
      k += 1;
      ans[k][1] = l;
      ans[k][2] = r;
      l = (r + 1);
    }
    write(k, "\n");
    {
      var i: dynamic = (1);
      while ((i <= (k)))
      {
        write(ans[i][1], " ", ans[i][2], "\n");
        i += 1;
      }
    }
  }
  return 0;
}
