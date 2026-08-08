// Translated from solution.cpp.

func read(num: dynamic)
{
  num = 0;
  var f = 1;
  var ch = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = 0;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    num = (((num * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  num = if (f) num else (-num);
}

func write(x: dynamic, ch: dynamic)
{
  var s = cpp_array(100);
  if ((x == 0))
  {
    putchar(cpp_char("0"));
    putchar(ch);
    return;
  }
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  var num = 0;
  while (x)
  {
    s[cpp_update(num, "++")] = ((x % 10));
    x = (x / 10);
  }
  {
    var i = ((num - 1));
    while ((i >= (0)))
    {
      putchar((s[i] + cpp_char("0")));
      i -= 1;
    }
  }
  putchar(ch);
}

var pi = acos(-1);

var eps = 1e-8;

func main()
{
  var ans = 100000000000000;
  var A = cpp_array(4);
  var ord = cpp_array(4);
  {
    var i = (1);
    while ((i <= (3)))
    {
      read(A[i]);
      ord[i] = i;
      i += 1;
    }
  }
  while (true)
  {
    var a = A[ord[1]];
    var b = A[ord[2]];
    var c = A[ord[3]];
    if ((b < c))
    {
      swap(b, c);
    }
    var res = c;
    a += c;
    b -= c;
    res += (((b / 2)) * 2);
    if ((b & 1))
    {
      res += a;
    }
    ans = min(ans, res);
    if (!((next_permutation((ord + 1), (ord + 4)))))
    {
      break;
    }
  }
  write(ans, cpp_char("\n"));
}
