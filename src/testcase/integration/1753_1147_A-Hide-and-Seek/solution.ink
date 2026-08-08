// Translated from solution.cpp.

func read()
{
  var xx = 0;
  var ff = 1;
  var ch = getchar();
  while (((ch > cpp_char("9")) || (ch < cpp_char("0"))))
  {
    if ((ch == cpp_char("-")))
    {
      ff = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    xx = (((xx * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (xx * ff);
}

func READ()
{
  var xx = 0;
  var ff = 1;
  var ch = getchar();
  while (((ch > cpp_char("9")) || (ch < cpp_char("0"))))
  {
    if ((ch == cpp_char("-")))
    {
      ff = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    xx = (((xx * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (xx * ff);
}

func one()
{
  var ch = getchar();
  while (((ch == cpp_char(" ")) || (ch == cpp_char("\n"))))
  {
    ch = getchar();
  }
  return ch;
}

var maxn = 100010;

var N: dynamic;

var K: dynamic;

var a = cpp_array(maxn);

var first = cpp_array(maxn);

var last = cpp_array(maxn);

func main()
{
  N = read();
  K = read();
  {
    var i = 1;
    while ((i <= K))
    {
      a[i] = read();
      if ((!first[a[i]]))
      {
        first[a[i]] = i;
      }
      last[a[i]] = i;
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 1;
    while ((i <= N))
    {
      if ((!first[i]))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < N))
    {
      if ((((first[i] > last[(i + 1)]) || (!first[i])) || (!last[(i + 1)])))
      {
        ans += 1;
      }
      i += 1;
    }
  }
  {
    var i = N;
    while ((i > 1))
    {
      if ((((first[i] > last[(i - 1)]) || (!first[i])) || (!last[(i - 1)])))
      {
        ans += 1;
      }
      i -= 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
