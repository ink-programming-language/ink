// Translated from solution.cpp.

class NS
{
  var MO: dynamic;
  var a: dynamic;
  var b: dynamic;
  func Normalize(x: dynamic)
  {
      x %= MO;
      if ((x < 0))
      {
        x += MO;
      }
    }
  func first()
  {
      return a;
    }
  func second()
  {
      return b;
    }
  func NS(a: dynamic = 0, b: dynamic = 0)
  {
      this->a = cpp_construct(a);
      this->b = cpp_construct(b);
      Normalize(this->a);
      Normalize(this->b);
    }
  func Powd(a: dynamic, b: dynamic)
  {
      assert((b >= 0));
      var answer = 1;
      {
        while ((b > 0))
        {
          if ((b & 1))
          {
            answer = ((answer * a) % MO);
          }
          a *= a;
          a %= MO;
          b >>= 1;
        }
      }
      return answer;
    }
  func Pair()
  {
      return NS(a, (-b));
    }
  func Inverse()
  {
      if (((!a) && (!b)))
      {
        write("0 has no INVERSE!!!", "\n");
        return NS();
      } else if ((b == 0))
      {
        return NS(Powd(a, (MO - 2)), 0);
      } else
      {
        var down = ((((a * a) % MO) + MO) - ((((BASE * b) % MO) * b) % MO));
        return (((*this)).Pair() * Powd((down % MO), (MO - 2)));
      }
    }
  func operator_subtract()
  {
      return NS((-a), (-b));
    }
  func operator_add(rhs: dynamic)
  {
      return NS((a + rhs.a), (b + rhs.b));
    }
  func operator_subtract(rhs: dynamic)
  {
      return NS((a - rhs.a), (b - rhs.b));
    }
  func operator_multiply(rhs: dynamic)
  {
      return NS((((a * rhs.a) % MO) + ((((b * rhs.b) % MO) * BASE) % MO)), (((a * rhs.b) % MO) + ((b * rhs.a) % MO)));
    }
  func operator_multiply(scale: dynamic)
  {
      return ((*this) * NS(scale, 0));
    }
  func operator_divide(rhs: dynamic)
  {
      return ((*this) * rhs.Inverse());
    }
}

var N = 222;

var Comb = cpp_array(N, N);

var S = cpp_array(N, N);

func Init()
{
  Comb[0][0] = cpp_assign(S[0][0], "=", 1);
  {
    var i = 1;
    while ((i < N))
    {
      Comb[i][0] = cpp_assign(Comb[i][i], "=", 1);
      S[i][0] = 0;
      S[i][i] = (S[(i - 1)][(i - 1)] * NS5(i).Inverse());
      {
        var j = 1;
        while ((j < i))
        {
          Comb[i][j] = (Comb[(i - 1)][j] + Comb[(i - 1)][(j - 1)]);
          var A = NS5(i).Inverse();
          var B = (A - 1);
          S[i][j] = ((S[(i - 1)][j] * B) + (S[(i - 1)][(j - 1)] * A));
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func Powd(a: dynamic, b: dynamic)
{
  assert((b >= 0));
  var answer = [1];
  {
    while ((b > 0))
    {
      if ((b & 1))
      {
        answer = (answer * a);
      }
      a = (a * a);
      b >>= 1;
    }
  }
  return answer;
}

func Calc(x: dynamic, start: dynamic, magic: dynamic)
{
  var result = (NS5(1) - Powd(magic, x));
  result = (start * result);
  result = (result * ((NS5(1) - magic)).Inverse());
  return result;
}

func SumK(x: dynamic, b: dynamic)
{
  var answer: dynamic;
  var magic = NS5(0, 1).Inverse();
  var P = (NS5(1, 1) / 2);
  var Q = P.Pair();
  {
    var i = 0;
    while ((i <= b))
    {
      var result = (Powd(NS5(-1), i) * Comb[b][i]);
      var start = (Powd(P, ((b - i))) * Powd(Q, i));
      if (((start.first() == 1) && (start.second() == 0)))
      {
        answer = (answer + ((result * start) * x));
        i += 1;
        continue;
      }
      answer = (answer + (result * Calc(x, start, start)));
      i += 1;
    }
  }
  return (answer * Powd(magic, b));
}

func Solve(x: dynamic, k: dynamic)
{
  var answer: dynamic;
  {
    var i = 0;
    while ((i <= k))
    {
      answer = (answer + (S[k][i] * SumK(x, i)));
      i += 1;
    }
  }
  return answer.first();
}

func main()
{
  var k: dynamic;
  var l: dynamic;
  var r: dynamic;
  Init();
  read(k, l, r);
  var answer = (Solve((r + 2), k) - Solve((l + 1), k));
  NS5.Normalize(answer);
  write(answer, "\n");
  return 0;
}
