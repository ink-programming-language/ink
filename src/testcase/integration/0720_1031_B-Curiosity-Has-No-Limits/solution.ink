// Translated from solution.cpp.

func main() -> dynamic
{
  cin.tie(0);
  ios_base.sync_with_stdio(0);
  var n: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_construct((n - 1));
  var b: dynamic = cpp_construct((n - 1));
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      read(b[i]);
      i += 1;
    }
  }
  {
    var t: dynamic = 0;
    while ((t < 4))
    {
      var ans: dynamic = cpp_uninitialized();
      ans.push_back(t);
      {
        var i: dynamic = 1;
        while ((i < n))
        {
          {
            var x: dynamic = 0;
            while ((x < 4))
            {
              if (((((ans.back() & x)) == b[(i - 1)]) && (((ans.back() | x)) == a[(i - 1)])))
              {
                ans.push_back(x);
                break;
              }
              x += 1;
            }
          }
          if ((ans.size() == i))
          {
            break;
          }
          i += 1;
        }
      }
      if ((ans.size() == n))
      {
        write("YES\n");
        {
          var i: dynamic = 0;
          while ((i < n))
          {
            write(ans[i], cpp_char(" "));
            i += 1;
          }
        }
        return 0;
      }
      t += 1;
    }
  }
  write("NO");
}
