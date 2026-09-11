// Translated from solution.cpp.

var pi: dynamic = acos(-1.0);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var vis: dynamic = cpp_array(300010);

var ans: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    scanf("%d %d", (&n), (&m));
    {
      var i: dynamic = 1;
      while ((i < ((3 * n) + 1)))
      {
        vis[i] = false;
        i += 1;
      }
    }
    ans.clear();
    var findans: dynamic = false;
    {
      var j: dynamic = 0;
      while ((j < m))
      {
        var u: dynamic = cpp_uninitialized();
        var v: dynamic = cpp_uninitialized();
        scanf("%d %d", (&u), (&v));
        if (findans)
        {
          j += 1;
          continue;
        }
        if (((!vis[u]) && (!vis[v])))
        {
          ans.push_back((j + 1));
          vis[u] = true;
          vis[v] = true;
          if ((ans.size() == n))
          {
            findans = true;
            printf("Matching\n");
            for (var item: dynamic in ans)
            {
              printf("%d ", item);
            }
            printf("\n");
          }
        }
        j += 1;
      }
    }
    if ((!findans))
    {
      printf("IndSet\n");
      var cnt: dynamic = 0;
      {
        var i: dynamic = 1;
        while ((i < ((3 * n) + 1)))
        {
          if ((vis[i] == false))
          {
            printf("%d ", i);
            cnt += 1;
            if ((cnt == n))
            {
              break;
            }
          }
          i += 1;
        }
      }
      printf("\n");
    }
  }
  return 0;
}
