// Translated from solution.cpp.

var Set: dynamic = cpp_uninitialized();

var maxn: dynamic = 4e18;

var S: dynamic = cpp_array(1000005);

var BIT: dynamic = cpp_array(64);

var cnt: dynamic = cpp_uninitialized();

func read() -> dynamic
{
  var c: dynamic = getchar();
  var ans: dynamic = 0;
  var flag: dynamic = true;
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
  return  (flag) ? ans : (-ans);
}

func Write(x: dynamic) -> dynamic
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

func min(x: dynamic, y: dynamic) -> dynamic
{
  return  ((x < y)) ? x : y;
}

func max(x: dynamic, y: dynamic) -> dynamic
{
  return  ((x > y)) ? x : y;
}

class oper
{
  var op: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

var ans: dynamic = cpp_uninitialized();

func Answer(OP: dynamic, X: dynamic, Y: dynamic) -> dynamic
{
  assert((Set.find(X) != Set.end()));
  assert((Set.find(Y) != Set.end()));
  var Z: dynamic = ( ((OP == 0)) ? (X + Y) : (X ^ Y));
  if ((Set.find(Z) == Set.end()))
  {
    ans.push_back([OP, X, Y]);
    Set.insert(Z);
    S[cpp_update(cnt, "++")] = Z;
  }
  return Z;
}

func Insert(x: dynamic) -> dynamic
{
  var tmp: dynamic = x;
  {
    var i: dynamic = 62;
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
          var j: dynamic = 62;
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

func calc(x: dynamic) -> dynamic
{
  Set.insert(x);
  S[1] = x;
  cnt = 1;
  Insert(x);
  var tmp: dynamic = x;
  while (((tmp * 2) <= maxn))
  {
    Answer(0, tmp, tmp);
    Insert((tmp * 2));
    tmp *= 2;
  }
  while (((cnt <= 99000) && (!BIT[0])))
  {
    var x: dynamic = S[((rand() % cnt) + 1)];
    var y: dynamic = S[((rand() % cnt) + 1)];
    var flag: dynamic = true;
    if (((x + y) > maxn))
    {
      continue;
    }
    Answer(0, x, y);
    Insert((x + y));
  }
  var CNT: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i <= 62))
    {
      CNT += (!(!S[i]));
      i += 1;
    }
  }
}

func main() -> dynamic
{
  Set.clear();
  cnt = 0;
  memset(BIT, 0, cpp_sizeof((BIT)));
  ans.clear();
  calc(read());
  assert((Set.find(1) != Set.end()));
  Write(ans.size());
  putchar(cpp_char("\n"));
  for (var i: dynamic in ans)
  {
    Write(i.x);
    putchar(cpp_char(" "));
    putchar( (i.op) ? cpp_char("^") : cpp_char("+"));
    putchar(cpp_char(" "));
    Write(i.y);
    putchar(cpp_char("\n"));
  }
  return 0;
}
