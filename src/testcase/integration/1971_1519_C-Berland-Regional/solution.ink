// Translated from solution.cpp.

func optimize() -> dynamic
{
  cpp_macro("ios_base::sync_with_stdio(0);");
}

var ll: dynamic = dynamic;

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic = cpp_uninitialized();
    var i: dynamic = cpp_uninitialized();
    var j: dynamic = cpp_uninitialized();
    var k: dynamic = 1;
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    var m: dynamic = cpp_uninitialized();
    var sum: dynamic = cpp_uninitialized();
    var a: dynamic = cpp_array(210000);
    var b: dynamic = cpp_array(210000);
    var c: dynamic = cpp_uninitialized();
    var mp1: dynamic = cpp_uninitialized();
    read(n);
    var v: dynamic = cpp_array((n + 3));
    var v1: dynamic = cpp_array((n + 3));
    var mp: dynamic = cpp_uninitialized();
    var p: dynamic = cpp_uninitialized();
    {
      i = 1;
      while ((i <= n))
      {
        read(a[i]);
        if ((mp1[a[i]] == 0))
        {
          b[k] = a[i];
          k += 1;
        }
        mp1[a[i]] = 1;
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= n))
      {
        read(c);
        v[a[i]].push_back(c);
        a[i] = 0;
        i += 1;
      }
    }
    sort((b + 1), (b + k));
    {
      i = 1;
      while ((i < k))
      {
        x = b[i];
        sum = 0;
        sort(v[x].rbegin(), v[x].rend());
        {
          j = 0;
          while ((j < v[x].size()))
          {
            sum += v[x][j];
            p = make_pair(x, (j + 1));
            mp[p] = sum;
            v1[(j + 1)].push_back(x);
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= n))
      {
        sum = 0;
        for (var u: dynamic in v1[i])
        {
          y = (v[u].size() / i);
          y = (y * i);
          p = make_pair(u, y);
          sum += mp[p];
        }
        write(sum, " ");
        i += 1;
      }
    }
    write("\n");
  }
  return 0;
}
