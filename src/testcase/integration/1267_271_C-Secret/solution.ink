// Translated from solution.cpp.

var N: dynamic = (1e6 + 10);

var ans: dynamic = cpp_array(N);

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  if ((n < (3 * k)))
  {
    write(-1, cpp_char("\n"));
    return 0;
  }
  if (((k % 2) == 0))
  {
    {
      var i: dynamic = 1;
      while ((i <= (3 * k)))
      {
        ans[i] = cpp_assign(ans[(i + 1)], "=", cpp_assign(ans[(i + 3)], "=", (2 * ((i / 6)))));
        ans[(i + 2)] = cpp_assign(ans[(i + 4)], "=", cpp_assign(ans[(i + 5)], "=", ((2 * ((i / 6))) + 1)));
        i += 6;
      }
    }
    {
      var i: dynamic = ((3 * k) + 1);
      while ((i <= n))
      {
        ans[i] = 0;
        i += 1;
      }
    }
  } else
  {
    {
      var i: dynamic = 1;
      while ((i <= (3 * k)))
      {
        ans[(i + 1)] = cpp_assign(ans[(i + 2)], "=", cpp_assign(ans[(i + 4)], "=", (2 * ((i / 6)))));
        ans[(i + 3)] = cpp_assign(ans[(i + 5)], "=", cpp_assign(ans[(i + 6)], "=", ((2 * ((i / 6))) + 1)));
        i += 6;
      }
    }
    {
      var i: dynamic = ((3 * k) + 1);
      while ((i <= n))
      {
        ans[i] = (k - 1);
        i += 1;
      }
    }
    ans[1] = (k - 1);
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      write((ans[i] + 1), cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
}
