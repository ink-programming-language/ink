// Translated from solution.cpp.

func optimize()
{
  cpp_macro("ios_base::sync_with_stdio(0);");
}

var ll = dynamic;

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var i: dynamic;
    var j: dynamic;
    var k = 1;
    var x: dynamic;
    var y: dynamic;
    var m: dynamic;
    var sum: dynamic;
    var a = cpp_array(210000);
    var b = cpp_array(210000);
    var c: dynamic;
    var mp1: dynamic;
    read(n);
    var v = cpp_array((n + 3));
    var v1 = cpp_array((n + 3));
    var mp: dynamic;
    var p: dynamic;
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
        for (var u in v1[i])
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
