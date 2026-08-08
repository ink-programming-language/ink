// Translated from solution.cpp.

func main()
{
  cin.tie(0);
  ios_base.sync_with_stdio(0);
  var n: dynamic;
  read(n);
  var a = cpp_construct((n - 1));
  var b = cpp_construct((n - 1));
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (n - 1)))
    {
      read(b[i]);
      i += 1;
    }
  }
  {
    var t = 0;
    while ((t < 4))
    {
      var ans: dynamic;
      ans.push_back(t);
      {
        var i = 1;
        while ((i < n))
        {
          {
            var x = 0;
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
          var i = 0;
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
