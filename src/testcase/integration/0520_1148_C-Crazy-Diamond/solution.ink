// Translated from solution.cpp.

var maxn = (cpp_cast(3e5) + 7);

var a = cpp_array(maxn);

var b = cpp_array(maxn);

var pos = cpp_array(maxn);

class node
{
  var x: dynamic;
  var y: dynamic;
}

var ans = cpp_array((maxn * 5));

func main()
{
  ios.sync_with_stdio(false);
  var n: dynamic;
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      pos[a[i]] = i;
      i += 1;
    }
  }
  var cnt = 0;
  {
    var i = 1;
    while ((i <= (n / 2)))
    {
      if ((pos[i] == i))
      {
        i += 1;
        continue;
      }
      if ((i == 1))
      {
        if ((pos[1] > (n / 2)))
        {
          ans[cpp_update(cnt, "++")] = [1, pos[1]];
          var temp = pos[1];
          swap(pos[a[1]], pos[1]);
          swap(a[temp], a[1]);
        } else
        {
          ans[cpp_update(cnt, "++")] = [1, n];
          ans[cpp_update(cnt, "++")] = [pos[1], n];
          ans[cpp_update(cnt, "++")] = [1, n];
          var temp = pos[1];
          swap(pos[a[1]], pos[1]);
          swap(a[temp], a[1]);
        }
        i += 1;
        continue;
      }
      if ((abs((pos[i] - i)) >= (n / 2)))
      {
        ans[cpp_update(cnt, "++")] = [i, pos[i]];
        var temp = pos[i];
        swap(pos[a[i]], pos[i]);
        swap(a[temp], a[i]);
      } else if ((pos[i] <= (n / 2)))
      {
        var temp = pos[i];
        ans[cpp_update(cnt, "++")] = [i, n];
        ans[cpp_update(cnt, "++")] = [pos[i], n];
        ans[cpp_update(cnt, "++")] = [i, n];
        swap(pos[a[i]], pos[i]);
        swap(a[temp], a[i]);
      } else
      {
        ans[cpp_update(cnt, "++")] = [pos[i], 1];
        ans[cpp_update(cnt, "++")] = [i, n];
        ans[cpp_update(cnt, "++")] = [1, n];
        ans[cpp_update(cnt, "++")] = [i, n];
        ans[cpp_update(cnt, "++")] = [pos[i], 1];
        var temp = pos[i];
        swap(pos[a[i]], pos[i]);
        swap(a[temp], a[i]);
      }
      i += 1;
    }
  }
  {
    var i = ((n / 2) + 1);
    while ((i <= n))
    {
      if ((pos[i] == i))
      {
        i += 1;
        continue;
      }
      ans[cpp_update(cnt, "++")] = [i, 1];
      ans[cpp_update(cnt, "++")] = [pos[i], 1];
      ans[cpp_update(cnt, "++")] = [i, 1];
      var temp = pos[i];
      swap(pos[a[i]], pos[i]);
      swap(a[temp], a[i]);
      i += 1;
    }
  }
  write(cnt, "\n");
  {
    var i = 1;
    while ((i <= cnt))
    {
      write(ans[i].x, " ", ans[i].y, "\n");
      i += 1;
    }
  }
  return 0;
}
