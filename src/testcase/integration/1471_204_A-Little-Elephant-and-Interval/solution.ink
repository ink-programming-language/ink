// Translated from solution.cpp.

var numbers: dynamic;

func get(A: dynamic)
{
  var B: dynamic;
  var c = 0;
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
    var v = 0;
    if ((A.size() > 2))
    {
      {
        var i = 1;
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
    var J = true;
    {
      var i = 1;
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
        var i = 1;
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

func works(A: dynamic)
{
  if ((A[0] == A[(A.size() - 1)]))
  {
    return true;
  }
  return false;
}

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(0);
  var l: dynamic;
  var r: dynamic;
  numbers.resize(20, 1);
  {
    var i = 1;
    while ((i < 20))
    {
      numbers[i] = (10 * numbers[(i - 1)]);
      i += 1;
    }
  }
  var A: dynamic;
  var B: dynamic;
  read(A, B);
  var a = get(A);
  var b = get(B);
  if (works(A))
  {
    write(((b - a) + 1), "\n");
  } else
  {
    write((b - a), "\n");
  }
  return 0;
}
