// Translated from solution.cpp.

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var first: dynamic = cpp_array(300005);

var l: dynamic = cpp_array(300005);

var ans: dynamic = cpp_array(300005);

var arr: dynamic = cpp_array(300005);

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
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
          var x: dynamic = first[i];
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
