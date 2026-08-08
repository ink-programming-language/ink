// Translated from solution.cpp.

var vis = cpp_array(1000000);

var n = cpp_array(1000000);

func main()
{
  var cnt = 0;
  var l: dynamic;
  read(l);
  {
    var i = 0;
    while ((i < l))
    {
      read(n[i]);
      i += 1;
    }
  }
  sort(n, (n + l));
  {
    var i = 0;
    while ((i < l))
    {
      var x = (n[i] - 1);
      if ((x < 1))
      {
        x = 1;
      }
      var y = n[i];
      var z = (n[i] + 1);
      if ((!vis[x]))
      {
        vis[x] = 1;
        cnt += 1;
      } else if ((!vis[y]))
      {
        vis[y] = 1;
        cnt += 1;
      } else if ((!vis[z]))
      {
        vis[z] = 1;
        cnt += 1;
      }
      i += 1;
    }
  }
  write(cnt, "\n");
}
