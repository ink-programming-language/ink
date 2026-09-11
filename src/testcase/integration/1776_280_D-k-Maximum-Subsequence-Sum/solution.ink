// Translated from solution.cpp.

func Get_Int() -> dynamic
{
  var Num: dynamic = 0;
  var Flag: dynamic = 1;
  var ch: dynamic = cpp_uninitialized();
  while (true)
  {
    ch = getchar();
    if ((ch == cpp_char("-")))
    {
      Flag = (-Flag);
    }
    if (!((((ch < cpp_char("0")) || (ch > cpp_char("9"))))))
    {
      break;
    }
  }
  while (true)
  {
    Num = (((Num * 10) + ch) - cpp_char("0"));
    ch = getchar();
    if (!((((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))))
    {
      break;
    }
  }
  return (Num * Flag);
}

class Data
{
  var Left: dynamic = cpp_uninitialized();
  var Right: dynamic = cpp_uninitialized();
  var Sum: dynamic = cpp_uninitialized();
  func operator_less(a: dynamic) -> dynamic
  {
      return (Sum < a.Sum);
    }
  func operator_add(a: dynamic) -> dynamic
  {
      return [Left, a.Right, (Sum + a.Sum)];
    }
}

class Node
{
  var LeftMax: dynamic = cpp_uninitialized();
  var RightMax: dynamic = cpp_uninitialized();
  var Max: dynamic = cpp_uninitialized();
  var LeftMin: dynamic = cpp_uninitialized();
  var RightMin: dynamic = cpp_uninitialized();
  var Min: dynamic = cpp_uninitialized();
  var Sum: dynamic = cpp_uninitialized();
  func operator_add(a: dynamic) -> dynamic
  {
      var x: dynamic = cpp_uninitialized();
      x.LeftMax = max(LeftMax, (Sum + a.LeftMax));
      x.RightMax = max(a.RightMax, (RightMax + a.Sum));
      x.Max = max(x.LeftMax, x.RightMax);
      x.Max = max(x.Max, Max);
      x.Max = max(x.Max, a.Max);
      x.Max = max(x.Max, (RightMax + a.LeftMax));
      x.LeftMin = min(LeftMin, (Sum + a.LeftMin));
      x.RightMin = min(a.RightMin, (RightMin + a.Sum));
      x.Min = min(x.LeftMin, x.RightMin);
      x.Min = min(x.Min, Min);
      x.Min = min(x.Min, a.Min);
      x.Min = min(x.Min, (RightMin + a.LeftMin));
      x.Sum = (Sum + a.Sum);
      return x;
    }
}

var A: dynamic = cpp_array((200005 * 4));

var Reverse: dynamic = cpp_array((200005 * 4));

func Push_Down(Now: dynamic) -> dynamic
{
  if (Reverse[Now])
  {
    swap(A[Now].LeftMax, A[Now].LeftMin);
    swap(A[Now].RightMax, A[Now].RightMin);
    swap(A[Now].Min, A[Now].Max);
    A[Now].LeftMax.Sum *= -1;
    A[Now].RightMax.Sum *= -1;
    A[Now].Max.Sum *= -1;
    A[Now].LeftMin.Sum *= -1;
    A[Now].RightMin.Sum *= -1;
    A[Now].Min.Sum *= -1;
    A[Now].Sum.Sum *= -1;
    Reverse[(Now << 1)] ^= 1;
    Reverse[((Now << 1) | 1)] ^= 1;
    Reverse[Now] = false;
  }
}

func Update(Now: dynamic) -> dynamic
{
  Push_Down((Now << 1));
  Push_Down(((Now << 1) | 1));
  A[Now] = (A[(Now << 1)] + A[((Now << 1) | 1)]);
}

func Build(Now: dynamic, Value: dynamic, Left: dynamic, Right: dynamic) -> dynamic
{
  if ((Left == Right))
  {
    A[Now].LeftMin = cpp_assign(A[Now].RightMin, "=", cpp_assign(A[Now].Min, "=", cpp_assign(A[Now].LeftMax, "=", cpp_assign(A[Now].RightMax, "=", cpp_assign(A[Now].Max, "=", cpp_assign(A[Now].Sum, "=", [Left, Left, Value[Left]]))))));
    return;
  }
  var Mid: dynamic = ((Left + Right) >> 1);
  Build((Now << 1), Value, ( (0) ? (Mid + 1) : Left), ( (0) ? Right : Mid));
  Build(((Now << 1) | 1), Value, ( (1) ? (Mid + 1) : Left), ( (1) ? Right : Mid));
  Update(Now);
}

func Modify(Now: dynamic, Position: dynamic, Value: dynamic, Left: dynamic, Right: dynamic) -> dynamic
{
  Push_Down(Now);
  if ((Left == Right))
  {
    A[Now].LeftMin = cpp_assign(A[Now].RightMin, "=", cpp_assign(A[Now].Min, "=", cpp_assign(A[Now].LeftMax, "=", cpp_assign(A[Now].RightMax, "=", cpp_assign(A[Now].Max, "=", cpp_assign(A[Now].Sum, "=", [Left, Left, Value]))))));
    return;
  }
  var Mid: dynamic = ((Left + Right) >> 1);
  var i: dynamic = (Position > Mid);
  Modify(((Now << 1) | i), Position, Value, ( (i) ? (Mid + 1) : Left), ( (i) ? Right : Mid));
  Update(Now);
}

func Flip(Now: dynamic, left: dynamic, right: dynamic, Left: dynamic, Right: dynamic) -> dynamic
{
  Push_Down(Now);
  if (((left == Left) && (right == Right)))
  {
    Reverse[Now] ^= 1;
    return;
  }
  var Mid: dynamic = ((Left + Right) >> 1);
  if (((left > Mid) || (right <= Mid)))
  {
    var i: dynamic = (left > Mid);
    Flip(((Now << 1) | i), left, right, ( (i) ? (Mid + 1) : Left), ( (i) ? Right : Mid));
  } else
  {
    Flip((Now << 1), left, Mid, ( (0) ? (Mid + 1) : Left), ( (0) ? Right : Mid));
    Flip(((Now << 1) | 1), (Mid + 1), right, ( (1) ? (Mid + 1) : Left), ( (1) ? Right : Mid));
  }
  Update(Now);
}

func Query(Now: dynamic, left: dynamic, right: dynamic, Left: dynamic, Right: dynamic) -> dynamic
{
  Push_Down(Now);
  if (((left == Left) && (right == Right)))
  {
    return A[Now];
  }
  var Mid: dynamic = ((Left + Right) >> 1);
  if (((left > Mid) || (right <= Mid)))
  {
    var i: dynamic = (left > Mid);
    return Query(((Now << 1) | i), left, right, ( (i) ? (Mid + 1) : Left), ( (i) ? Right : Mid));
  }
  return (Query((Now << 1), left, Mid, ( (0) ? (Mid + 1) : Left), ( (0) ? Right : Mid)) + Query(((Now << 1) | 1), (Mid + 1), right, ( (1) ? (Mid + 1) : Left), ( (1) ? Right : Mid)));
}

var N: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

var Top: dynamic = cpp_uninitialized();

var Value: dynamic = cpp_array(200005);

var temp1: dynamic = cpp_array(200005);

var temp2: dynamic = cpp_array(200005);

func main() -> dynamic
{
  read(N);
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      Value[i] = Get_Int();
      i += 1;
    }
  }
  Segment_Tree.Build(1, Value, 1, N);
  read(Q);
  while (cpp_update(Q, "--"))
  {
    if (Get_Int())
    {
      var Left: dynamic = Get_Int();
      var Right: dynamic = Get_Int();
      var K: dynamic = Get_Int();
      var Ans: dynamic = 0;
      while (cpp_update(K, "--"))
      {
        var Now: dynamic = Segment_Tree.Query(1, Left, Right, 1, N);
        if ((Now.Max.Sum <= 0))
        {
          break;
        }
        Ans += Now.Max.Sum;
        temp1[cpp_update(Top, "++")] = Now.Max.Left;
        temp2[Top] = Now.Max.Right;
        Segment_Tree.Flip(1, Now.Max.Left, Now.Max.Right, 1, N);
      }
      while (Top)
      {
        Segment_Tree.Flip(1, temp1[Top], temp2[Top], 1, N);
        Top -= 1;
      }
      printf("%lld\n", max(Ans, 0));
    } else
    {
      var x: dynamic = Get_Int();
      var y: dynamic = Get_Int();
      Segment_Tree.Modify(1, x, y, 1, N);
    }
  }
  return 0;
}
