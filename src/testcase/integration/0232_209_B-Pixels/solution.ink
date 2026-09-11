// Translated from solution.cpp.

func read(num: dynamic) -> dynamic
{
  num = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
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
  num =  (f) ? num : (-num);
}

func write(x: dynamic, ch: dynamic) -> dynamic
{
  var s: dynamic = cpp_array(100);
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
  var num: dynamic = 0;
  while (x)
  {
    s[cpp_update(num, "++")] = ((x % 10));
    x = (x / 10);
  }
  {
    var i: dynamic = ((num - 1));
    while ((i >= (0)))
    {
      putchar((s[i] + cpp_char("0")));
      i -= 1;
    }
  }
  putchar(ch);
}

var pi: dynamic = acos(-1);

var eps: dynamic = 1e-8;

func main() -> dynamic
{
  var ans: dynamic = 100000000000000;
  var A: dynamic = cpp_array(4);
  var ord: dynamic = cpp_array(4);
  {
    var i: dynamic = (1);
    while ((i <= (3)))
    {
      read(A[i]);
      ord[i] = i;
      i += 1;
    }
  }
  while (true)
  {
    var a: dynamic = A[ord[1]];
    var b: dynamic = A[ord[2]];
    var c: dynamic = A[ord[3]];
    if ((b < c))
    {
      swap(b, c);
    }
    var res: dynamic = c;
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
