// Translated from solution.cpp.

var pi = acos(-1.0);

var n: dynamic;

var m: dynamic;

var vis = cpp_array(300010);

var ans: dynamic;

func main()
{
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    scanf("%d %d", (&n), (&m));
    {
      var i = 1;
      while ((i < ((3 * n) + 1)))
      {
        vis[i] = false;
        i += 1;
      }
    }
    ans.clear();
    var findans = false;
    {
      var j = 0;
      while ((j < m))
      {
        var u: dynamic;
        var v: dynamic;
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
            for (var item in ans)
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
      var cnt = 0;
      {
        var i = 1;
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
