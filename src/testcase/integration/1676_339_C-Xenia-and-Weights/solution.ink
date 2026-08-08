// Translated from solution.cpp.

func read(x: dynamic)
{
  var c = getchar();
  var f = 0;
  x = 0;
  while ((!isdigit(c)))
  {
    f |= (c == cpp_char("-"));
    c = getchar();
  }
  while (isdigit(c))
  {
    x = ((((x << 1)) + ((x << 3))) + ((c ^ 48)));
    c = getchar();
  }
  if (f)
  {
    x = (-x);
  }
  return x;
}

func write(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    write((-x));
  } else
  {
    if ((x > 9))
    {
      write((x / 10));
    }
    putchar((cpp_char("0") + (x % 10)));
  }
}

var ans = cpp_array(1005);

var m: dynamic;

var s = cpp_array(11);

func dfs(last: dynamic, num: dynamic, x: dynamic)
{
  if ((x == (m + 1)))
  {
    puts("YES");
    {
      var i = 1;
      while ((i <= m))
      {
        write(ans[i]);
        putchar(cpp_char(" "));
        i += 1;
      }
    }
    exit(0);
  }
  {
    var i = (num + 1);
    while ((i <= 10))
    {
      if (((i != last) && (s[i] == cpp_char("1"))))
      {
        ans[x] = i;
        dfs(i, (i - num), (x + 1));
      }
      i += 1;
    }
  }
}

func main()
{
  scanf("%s", (s + 1));
  read(m);
  dfs(0, 0, 1);
  puts("NO");
}
