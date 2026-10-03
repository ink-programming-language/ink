// A single leading dot resolves provider relative to this module's package directory.
from .provider import value as siblingValue;

// Re-export behavior is unnecessary: the local public wrapper calls the imported sibling.
func answer(): i32
{
  return siblingValue() + 2;
}
