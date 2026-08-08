// Translated from solution.cpp.

var p: dynamic;

var q: dynamic;

func rev(x: dynamic)
{
  var ret = 0;
  while (x)
  {
    ret *= 10;
    ret += (x % 10);
    x /= 10;
  }
  return ret;
}

var vis = cpp_array((4000000 + 10));

func init()
{
  vis[1] = 1;
  {
    var i = 2;
    while ((i <= 4000000))
    {
      if ((!vis[i]))
      {
        {
          var j = (i + i);
          while ((j <= 4000000))
          {
            vis[j] = 1;
            j += i;
          }
        }
      }
      i += 1;
    }
  }
}

func main()
{
  var ans = -1;
  var A = 0;
  var B = 0;
  scanf("%d%d", (&p), (&q));
  init();
  {
    var i = 1;
    while ((i <= 4000000))
    {
      if ((i == rev(i)))
      {
        B += 1;
      }
      if ((!vis[i]))
      {
        A += 1;
      }
      if (((q * cpp_cast(A)) <= (p * cpp_cast(B))))
      {
        ans = i;
      }
      i += 1;
    }
  }
  if ((ans == -1))
  {
    puts("Palindromic tree is better than splay tree");
  } else
  {
    printf("%d", ans);
  }
}
