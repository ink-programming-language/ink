// Translated from solution.cpp.

class debugger
{
  func operator(v: dynamic) -> dynamic
  {
      write(v, " ");
      return (*self);
    }
}

var dbg: dynamic = cpp_uninitialized();

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  a = ( (((a) < 0)) ? (-(a)) : (a));
  b = ( (((b) < 0)) ? (-(b)) : (b));
  while (b)
  {
    a = (a % b);
    swap(a, b);
  }
  return a;
}

func ext_gcd(A: dynamic, B: dynamic, X: dynamic, Y: dynamic) -> dynamic
{
  var x2: dynamic = cpp_uninitialized();
  var y2: dynamic = cpp_uninitialized();
  var x1: dynamic = cpp_uninitialized();
  var y1: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var r2: dynamic = cpp_uninitialized();
  var r1: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
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

func modInv(a: dynamic, m: dynamic) -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  ext_gcd(a, m, (&x), (&y));
  x %= m;
  if ((x < 0))
  {
    x += m;
  }
  return x;
}

func bigmod(a: dynamic, p: dynamic, m: dynamic) -> dynamic
{
  var res: dynamic = (1 % m);
  var x: dynamic = (a % m);
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

var inf: dynamic = 2147383647;

var mod: dynamic = 1000000007;

var pi: dynamic = (2 * acos(0.0));

var eps: dynamic = 1e-11;

var myStack: dynamic = cpp_uninitialized();

var myVec: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  while ((!myStack.empty()))
  {
    myStack.pop();
  }
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      read(a);
      myVec.push_back(a);
      i += 1;
    }
  }
  var flag: dynamic = 1;
  var mx: dynamic = -1;
  {
    var i: dynamic = 0;
    while ((i < myVec.size()))
    {
      var curr: dynamic = myVec[i];
      if (myStack.empty())
      {
        myStack.push(curr);
      } else
      {
        var top: dynamic = myStack.top();
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
