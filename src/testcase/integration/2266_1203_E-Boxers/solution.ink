// Translated from solution.cpp.

var vis: dynamic = cpp_array(1000000);

var n: dynamic = cpp_array(1000000);

func main() -> dynamic
{
  var cnt: dynamic = 0;
  var l: dynamic = cpp_uninitialized();
  read(l);
  {
    var i: dynamic = 0;
    while ((i < l))
    {
      read(n[i]);
      i += 1;
    }
  }
  sort(n, (n + l));
  {
    var i: dynamic = 0;
    while ((i < l))
    {
      var x: dynamic = (n[i] - 1);
      if ((x < 1))
      {
        x = 1;
      }
      var y: dynamic = n[i];
      var z: dynamic = (n[i] + 1);
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
