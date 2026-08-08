// Translated from solution.cpp.

class debugger
{
  func operator(v: dynamic)
  {
      write(v, " ");
      return (*this);
    }
}

var dbg: dynamic;

func gcd(a: dynamic, b: dynamic)
{
  a = (if (((a) < 0)) (-(a)) else (a));
  b = (if (((b) < 0)) (-(b)) else (b));
  while (b)
  {
    a = (a % b);
    swap(a, b);
  }
  return a;
}

func ext_gcd(A: dynamic, B: dynamic, X: dynamic, Y: dynamic)
{
  var x2: dynamic;
  var y2: dynamic;
  var x1: dynamic;
  var y1: dynamic;
  var x: dynamic;
  var y: dynamic;
  var r2: dynamic;
  var r1: dynamic;
  var q: dynamic;
  var r: dynamic;
  x2 = 1;
  y2 = 0;
  x1 = 0;
  y1 = 1;
  {
    r2 = A;
    r1 = B;
    while ((r1 != 0))
    {
      q = (r2 / r1);
      r = (r2 % r1);
      x = (x2 - ((q * x1)));
      y = (y2 - ((q * y1)));
      r2 = r1;
      r1 = r;
      x2 = x1;
      y2 = y1;
      x1 = x;
      y1 = y;
    }
  }
  (*X) = x2;
  (*Y) = y2;
  return r2;
}

func modInv(a: dynamic, m: dynamic)
{
  var x: dynamic;
  var y: dynamic;
  ext_gcd(a, m, (&x), (&y));
  x %= m;
  if ((x < 0))
  {
    x += m;
  }
  return x;
}

func bigmod(a: dynamic, p: dynamic, m: dynamic)
{
  var res = (1 % m);
  var x = (a % m);
  while (p)
  {
    if ((p & 1))
    {
      res = (((res * x)) % m);
    }
    x = (((x * x)) % m);
    p >>= 1;
  }
  return res;
}

var inf = 2147383647;

var mod = 1000000007;

var pi = (2 * acos(0.0));

var eps = 1e-11;

var myStack: dynamic;

var myVec: dynamic;

func main()
{
  while ((!myStack.empty()))
  {
    myStack.pop();
  }
  var n: dynamic;
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      var a: dynamic;
      read(a);
      myVec.push_back(a);
      i += 1;
    }
  }
  var flag = 1;
  var mx = -1;
  {
    var i = 0;
    while ((i < myVec.size()))
    {
      var curr = myVec[i];
      if (myStack.empty())
      {
        myStack.push(curr);
      } else
      {
        var top = myStack.top();
        if ((top == curr))
        {
          myStack.pop();
          if ((curr > mx))
          {
            mx = curr;
          }
        } else if ((top > curr))
        {
          myStack.push(curr);
        } else
        {
          flag = 0;
          break;
        }
      }
      i += 1;
    }
  }
  if ((!flag))
  {
    write("NO\n");
  } else if ((myStack.size() > 1))
  {
    write("NO\n");
  } else
  {
    if ((myStack.size() == 0))
    {
      write("YES\n");
    } else if (((myStack.size() == 1) && (myStack.top() >= mx)))
    {
      write("YES\n");
    } else
    {
      write("NO\n");
    }
  }
  return 0;
}
