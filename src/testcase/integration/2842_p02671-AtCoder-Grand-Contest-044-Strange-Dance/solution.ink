// Translated from solution.cpp.

var mo = cpp_expression("#include<");

func read()
{
  var xx = 0;
  var flagg = 1;
  var ch = getchar();
  while (((((ch < cpp_char("0")) || (ch > cpp_char("9")))) && (ch != cpp_char("-"))))
  {
    ch = getchar();
  }
  if ((ch == cpp_char("-")))
  {
    flagg = -1;
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    xx = (((xx * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (xx * flagg);
}

func pus(xx: dynamic, flagg: dynamic)
{
  if ((xx < 0))
  {
    putchar(cpp_char("-"));
    xx = (-xx);
  }
  if ((xx >= 10))
  {
    pus((xx / 10), 0);
  }
  putchar(((xx % 10) + cpp_char("0")));
  if ((flagg == 1))
  {
    putchar(cpp_char(" "));
  }
  if ((flagg == 2))
  {
    putchar(cpp_char("\n"));
  }
  return;
}

var n: dynamic;

var m: dynamic;

var len: dynamic;

var i: dynamic;

var x: dynamic;

var y: dynamic;

var a = cpp_array(1500005);

var flag = cpp_array(1500005);

var ch = cpp_array(3, 1500005);

var top: dynamic;

var ans = cpp_array(1500005);

var s = cpp_array(200005);

func buildtree(v: dynamic, w: dynamic, ww: dynamic)
{
  if ((w == m))
  {
    a[v] = ww;
    return;
  }
  top += 1;
  ch[v][0] = top;
  top += 1;
  ch[v][1] = top;
  top += 1;
  ch[v][2] = top;
  buildtree(ch[v][0], (w * 3), ww);
  buildtree(ch[v][1], (w * 3), (ww + w));
  buildtree(ch[v][2], (w * 3), (ww + (w * 2)));
}

func ytree(v: dynamic, w: dynamic, ww: dynamic)
{
  if ((w == m))
  {
    ans[a[v]] = ww;
    return;
  }
  if ((flag[v] == 1))
  {
    swap(ch[v][1], ch[v][2]);
    flag[ch[v][0]] ^= 1;
    flag[ch[v][1]] ^= 1;
    flag[ch[v][2]] ^= 1;
  }
  ytree(ch[v][0], (w * 3), ww);
  ytree(ch[v][1], (w * 3), (ww + w));
  ytree(ch[v][2], (w * 3), (ww + (w * 2)));
}

func main()
{
  n = read();
  scanf("%s", (s + 1));
  len = strlen((s + 1));
  m = 1;
  {
    i = 1;
    while ((i <= n))
    {
      m *= 3;
      i += 1;
    }
  }
  buildtree(0, 1, 0);
  {
    i = 1;
    while ((i <= len))
    {
      if ((s[i] == cpp_char("S")))
      {
        flag[0] ^= 1;
      } else
      {
        x = 0;
        while ((ch[x][0] != 0))
        {
          if ((flag[x] == 1))
          {
            swap(ch[x][1], ch[x][2]);
            flag[ch[x][0]] ^= 1;
            flag[ch[x][1]] ^= 1;
            flag[ch[x][2]] ^= 1;
            flag[x] = 0;
          }
          y = ch[x][2];
          ch[x][2] = ch[x][1];
          ch[x][1] = ch[x][0];
          ch[x][0] = y;
          x = y;
        }
      }
      i += 1;
    }
  }
  ytree(0, 1, 0);
  {
    i = 0;
    while ((i < m))
    {
      pus(ans[i], 1);
      i += 1;
    }
  }
  write("\n");
  return 0;
}
