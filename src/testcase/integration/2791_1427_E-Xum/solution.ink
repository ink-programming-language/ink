// Translated from solution.cpp.

var Set: dynamic;

var maxn = 4e18;

var S = cpp_array(1000005);

var BIT = cpp_array(64);

var cnt: dynamic;

func read()
{
  var c = getchar();
  var ans = 0;
  var flag = true;
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    flag &= ((c != cpp_char("-")));
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    ans = (((ans * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return if (flag) ans else (-ans);
}

func Write(x: dynamic)
{
  if ((x < 10))
  {
    putchar((x + cpp_char("0")));
  } else
  {
    Write((x / 10));
    putchar(((x % 10) + cpp_char("0")));
  }
}

func min(x: dynamic, y: dynamic)
{
  return if ((x < y)) x else y;
}

func max(x: dynamic, y: dynamic)
{
  return if ((x > y)) x else y;
}

class oper
{
  var op: dynamic;
  var x: dynamic;
  var y: dynamic;
}

var ans: dynamic;

func Answer(OP: dynamic, X: dynamic, Y: dynamic)
{
  assert((Set.find(X) != Set.end()));
  assert((Set.find(Y) != Set.end()));
  var Z = (if ((OP == 0)) (X + Y) else (X ^ Y));
  if ((Set.find(Z) == Set.end()))
  {
    ans.push_back([OP, X, Y]);
    Set.insert(Z);
    S[cpp_update(cnt, "++")] = Z;
  }
  return Z;
}

func Insert(x: dynamic)
{
  var tmp = x;
  {
    var i = 62;
    while ((i >= 0))
    {
      if ((!(((x >> i) & 1))))
      {
        i -= 1;
        continue;
      }
      if ((!BIT[i]))
      {
        x = tmp;
        {
          var j = 62;
          while ((j >= i))
          {
            if ((BIT[j] && (((x >> j) & 1))))
            {
              Answer(1, x, BIT[j]);
              x ^= BIT[j];
            }
            j -= 1;
          }
        }
        BIT[i] = x;
        return true;
      } else
      {
        x ^= BIT[i];
      }
      i -= 1;
    }
  }
  return false;
}

func calc(x: dynamic)
{
  Set.insert(x);
  S[1] = x;
  cnt = 1;
  Insert(x);
  var tmp = x;
  while (((tmp * 2) <= maxn))
  {
    Answer(0, tmp, tmp);
    Insert((tmp * 2));
    tmp *= 2;
  }
  while (((cnt <= 99000) && (!BIT[0])))
  {
    var x = S[((rand() % cnt) + 1)];
    var y = S[((rand() % cnt) + 1)];
    var flag = true;
    if (((x + y) > maxn))
    {
      continue;
    }
    Answer(0, x, y);
    Insert((x + y));
  }
  var CNT = 0;
  {
    var i = 0;
    while ((i <= 62))
    {
      CNT += (!(!S[i]));
      i += 1;
    }
  }
}

func main()
{
  Set.clear();
  cnt = 0;
  memset(BIT, 0, cpp_sizeof((BIT)));
  ans.clear();
  calc(read());
  assert((Set.find(1) != Set.end()));
  Write(ans.size());
  putchar(cpp_char("\n"));
  for (var i in ans)
  {
    Write(i.x);
    putchar(cpp_char(" "));
    putchar(if (i.op) cpp_char("^") else cpp_char("+"));
    putchar(cpp_char(" "));
    Write(i.y);
    putchar(cpp_char("\n"));
  }
  return 0;
}
