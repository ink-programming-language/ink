// Translated from solution.cpp.

var numbers: dynamic = cpp_uninitialized();

func get(A: dynamic) -> dynamic
{
  var B: dynamic = cpp_uninitialized();
  var c: dynamic = 0;
  B.push_back(cpp_char("1"));
  while ((B.size() < A.size()))
  {
    while ((B[0] <= cpp_char("9")))
    {
      if ((B.size() > 1))
      {
        c += (numbers[(B.size() - 2)]);
      } else
      {
        c += 1;
      }
      B[0] += 1;
    }
    B.push_back(cpp_char("1"));
    B[0] = cpp_char("1");
  }
  while ((B[0] < A[0]))
  {
    if ((B.size() > 1))
    {
      c += (numbers[(B.size() - 2)]);
    } else
    {
      c += 1;
    }
    B[0] += 1;
  }
  if ((A[0] <= A[(A.size() - 1)]))
  {
    var v: dynamic = 0;
    if ((A.size() > 2))
    {
      {
        var i: dynamic = 1;
        while ((i < (A.size() - 1)))
        {
          v *= 10;
          v += ((A[i] - cpp_char("0")));
          i += 1;
        }
      }
      v += 1;
    }
    if ((A.size() <= 2))
    {
      if ((A[0] <= A[(A.size() - 1)]))
      {
        v += 1;
      }
    }
    return (c + v);
  }
  if ((A[0] > A[(A.size() - 1)]))
  {
    var J: dynamic = true;
    {
      var i: dynamic = 1;
      while ((i < (A.size() - 1)))
      {
        if ((A[i] != cpp_char("0")))
        {
          J = false;
        }
        i += 1;
      }
    }
    if ((!J))
    {
      {
        var i: dynamic = 1;
        while ((i < (A.size() - 1)))
        {
          c += ((((A[i] - cpp_char("0"))) * numbers[((A.size() - i) - 2)]));
          i += 1;
        }
      }
      return c;
    } else
    {
      return c;
    }
  }
}

func works(A: dynamic) -> dynamic
{
  if ((A[0] == A[(A.size() - 1)]))
  {
    return true;
  }
  return false;
}

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  numbers.resize(20, 1);
  {
    var i: dynamic = 1;
    while ((i < 20))
    {
      numbers[i] = (10 * numbers[(i - 1)]);
      i += 1;
    }
  }
  var A: dynamic = cpp_uninitialized();
  var B: dynamic = cpp_uninitialized();
  read(A, B);
  var a: dynamic = get(A);
  var b: dynamic = get(B);
  if (works(A))
  {
    write(((b - a) + 1), "\n");
  } else
  {
    write((b - a), "\n");
  }
  return 0;
}
