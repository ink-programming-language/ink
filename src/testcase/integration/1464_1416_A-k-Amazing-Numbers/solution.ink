// Translated from solution.cpp.

var i: dynamic;

var j: dynamic;

var k: dynamic;

var first = cpp_array(300005);

var l = cpp_array(300005);

var ans = cpp_array(300005);

var arr = cpp_array(300005);

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    read(n);
    {
      i = 1;
      while ((i < (n + 1)))
      {
        read(arr[i]);
        first[i] = cpp_assign(l[i], "=", 0);
        ans[i] = -1;
        i += 1;
      }
    }
    {
      i = 1;
      while ((i < (n + 1)))
      {
        first[arr[i]] = max(first[arr[i]], (i - l[arr[i]]));
        l[arr[i]] = i;
        i += 1;
      }
    }
    {
      i = 1;
      while ((i < (n + 1)))
      {
        first[i] = max(first[i], ((n + 1) - l[i]));
        {
          var x = first[i];
          while (((x <= n) && (ans[x] == -1)))
          {
            ans[x] = i;
            x += 1;
          }
        }
        i += 1;
      }
    }
    {
      i = 1;
      while ((i < (n + 1)))
      {
        write(ans[i], " ");
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
