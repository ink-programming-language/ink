// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(200005);

var fx: dynamic = cpp_array(5000005);

var fy: dynamic = cpp_array(5000005);

var lst: dynamic = cpp_uninitialized();

var v: dynamic = cpp_array(2500005);

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      v[a[i]].push_back(i);
      if ((v[a[i]].size() == 4))
      {
        puts("YES");
        for (var j: dynamic in v[a[i]])
        {
          write(j, cpp_char(" "));
        }
        return 0;
      }
      if ((v[a[i]].size() == 2))
      {
        if (lst)
        {
          printf("YES\n%d %d %d %d\n", v[a[i]][0], v[lst][0], v[a[i]][1], v[lst][1]);
          exit(0);
        } else
        {
          lst = a[i];
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j < i))
        {
          if ((!fx[(a[i] + a[j])]))
          {
            fx[(a[i] + a[j])] = i;
            fy[(a[i] + a[j])] = j;
          } else
          {
            if (((((i != fx[(a[i] + a[j])]) && (j != fx[(a[i] + a[j])])) && (i != fy[(a[i] + a[j])])) && (j != fy[(a[i] + a[j])])))
            {
              printf("YES\n%d %d %d %d\n", i, j, fx[(a[i] + a[j])], fy[(a[i] + a[j])]);
              return 0;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  puts("NO");
  return 0;
}
